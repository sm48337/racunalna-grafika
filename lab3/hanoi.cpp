#include <GL/glut.h>
#include <glm/glm.hpp>
#include <array>
#include <deque>
#include <iostream>
#include <numbers>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define NUM_DISKS 8
#define DISK_HEIGHT 0.4
#define SPEED 100
#define MAX_SPEED 1000

using solution = std::pair<size_t, size_t>;

struct Disk {
    glm::vec3 position;
    glm::vec3 normal = glm::vec3(0.0, 0.0, 1.0);
};

int active_disk_index;
glm::vec3 start_pos, dest_pos;
double u, step_u;
bool is_moving;
int direction;

struct Rod {
    std::array<glm::vec3, NUM_DISKS> positions;
    std::array<int, NUM_DISKS> occupied;
};

struct GameBoard {
    double x_min, y_min, x_max, y_max;
    double base_radius;
    std::array<Rod, 3> rods;
};

std::array<Disk, NUM_DISKS> disks;
GameBoard board;

std::deque<solution> solutions;
bool do_solve = false;

int speed_modifier = SPEED;
int moves = 0;
int prev_time = 0;
int window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;

void initialize_game() {
    board.base_radius = 1.0;
    board.x_min = 0.0;
    board.x_max = 10 * board.base_radius;
    board.y_min = 0.0;
    board.y_max = 5 * board.base_radius;

    double x_center = (board.x_max - board.x_min) / 2.0;
    double y_center = (board.y_max - board.y_min) / 2.0;

    double dx = (board.x_max - board.x_min) / 3;

    for (size_t i = 0; i < board.rods.size(); i++) {
        for (int h = 0; h < NUM_DISKS; h++) {
            if (i == 0) {
                board.rods[i].occupied[h] = NUM_DISKS - 1 - h;
            }
            else {
                board.rods[i].occupied[h] = -1;
            }
        }
    }

    for (int i = 0; i < 3; i ++) {
        for (int h = 0; h < NUM_DISKS; h++) {
            double x = x_center + ((int)i - 1) * dx;
            double y = y_center;
            double z = (h + 1) * 0.35;
            glm::vec3 &pos_to_set = board.rods[i].positions[h];
            pos_to_set.x = x;
            pos_to_set.y = y;
            pos_to_set.z = z;
        }
    }

    for (size_t i = 0; i < NUM_DISKS; i++) {
        disks[i].position = board.rods[0].positions[NUM_DISKS - i - 1];
    }

    active_disk_index = -1;
    is_moving = false;
    step_u = 0.015;
    u = 0.0;
    direction = 0;
}

void initialize() {
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);

    GLfloat light0_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    initialize_game();
}

void draw_solid_cylinder(double x, double y, double z, double r, double h) {
    GLUquadric* q = gluNewQuadric();
    GLint slices = 50;
    GLint stacks = 10;

    glPushMatrix();
    glTranslatef(x, y, z);
    gluCylinder(q, r, r, h, slices, stacks);
    glTranslatef(0, 0, h);
    gluDisk(q, 0, r, slices, stacks);
    glPopMatrix();
    gluDeleteQuadric(q);
}

void draw_board_and_rods(const GameBoard &board) {
    GLfloat table_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat rod_color[] = { 0.6f, 0.6f, 0.6f, 1.0f };

    glPushMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, table_color);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex2f(board.x_min, board.y_min);
        glVertex2f(board.x_min, board.y_max);
        glVertex2f(board.x_max, board.y_max);
        glVertex2f(board.x_max, board.y_min);
    glEnd();

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, rod_color);

    double r = board.base_radius;
    for (int i = 0; i < 3; i++) {
        glm::vec3 const& p = board.rods[i].positions[0];
        draw_solid_cylinder(p.x, p.y, 0, r * 0.1, (NUM_DISKS + 1) * DISK_HEIGHT - 0.1);
        draw_solid_cylinder(p.x, p.y, 0, r, 0.1);
    }
    glPopMatrix();
}

void draw_disks() {
    double radius;
    GLfloat material[] = { 0.1f, 0.1f, 0.6f, 1.0f };
    for (int i = 0; i < NUM_DISKS; i++) {
        const auto &disk = disks[i];

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);

        radius = double(i + 2) / (NUM_DISKS + 1) * board.base_radius;
        int d = direction;

        glPushMatrix();
        glTranslatef(disk.position.x, disk.position.y, disk.position.z);
        double theta = acos(disk.normal.z);
        theta *= 180.0f / std::numbers::pi;
        glRotatef(d * theta , 0.0f, 1.0f, 0.0f);
        draw_solid_cylinder(0, 0, 0.1 - DISK_HEIGHT, radius, DISK_HEIGHT);
        glPopMatrix();
    }
}

void display_handler() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double x_center = (board.x_max - board.x_min) / 2.0;
    double y_center = (board.y_max - board.y_min) / 2.0;
    double r = board.base_radius;

    static float view[] = {0, 0, 0};
    view[0] = x_center;
    view[1] = y_center - 10;
    view[2] = 3 * r;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        view[0], view[1], view[2],
        x_center, y_center, 3.0,
        0.0, 0.0, 1.0
    );

    glPushMatrix();
        draw_board_and_rods(board);
        draw_disks();
    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}

void reshape_handler(int w, int h) {
    window_width = w;
    window_height = h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void move_stack(int n, int from, int to) {
    if (n == 1) {
        solutions.push_back({from, to});
        moves++;
        return;
    }
    move_stack(n - 1, from, 3 - to - from);
    move_stack(1, from, to);
    move_stack(n - 1, 3 - to - from, to);
}

void solve() {
    move_stack(NUM_DISKS, 0, 2);
}

void keyboard_handler(unsigned char key, int, int) {
    switch (key) {
    case 27:
    case 'q':
    case 'Q':
        exit(0);
        break;

    case 's':
    case 'S':
        if (board.rods[0].occupied[NUM_DISKS - 1] < 0)
            break;
        solve();
        do_solve = true;
        break;

    case '+':
        speed_modifier = std::clamp(speed_modifier + 20, 1, MAX_SPEED);
        break;
    case '-':
        speed_modifier = std::clamp(speed_modifier - 20, 1, MAX_SPEED);
        break;

    default:
        break;
    };
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, height, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1,1,0);
    glutSwapBuffers();
}

void move_disk(int from_rod, int to_rod) {
    int d = to_rod - from_rod;

    if (d > 0)
        direction = 1;
    else if (d < 0)
        direction = -1;

    if ((from_rod == to_rod ) || (from_rod < 0) || (to_rod < 0) || (from_rod > 2) || (to_rod > 2) )
        return;

    int i;
    for (i = NUM_DISKS - 1; i >= 0 && board.rods[from_rod].occupied[i] < 0; i--);
    if ((i < 0) || (i == 0 && board.rods[from_rod].occupied[i] < 0) )
        return;

    start_pos = board.rods[from_rod].positions[i];

    active_disk_index = board.rods[from_rod].occupied[i];
    is_moving = true;
    u = 0.0;

    int j;
    for (j = 0; j < NUM_DISKS - 1 && board.rods[to_rod].occupied[j] >= 0; j++);
    dest_pos = board.rods[to_rod].positions[j];

    board.rods[from_rod].occupied[i] = -1;
    board.rods[to_rod].occupied[j] = active_disk_index;
}

bool raise_disk() {
    if (u == 0.0 && (disks[active_disk_index].position.z < (NUM_DISKS + 1) * DISK_HEIGHT + 0.2 * (board.base_radius)) ) {
        disks[active_disk_index].position.z += 0.05;
        return false;
    }
    return true;
}

bool move_disk_to_rod() {
        int index = active_disk_index;
        if (disks[index].position.x < dest_pos.x) {
            disks[index].position.x += 0.05;
        } else if (disks[index].position.x > dest_pos.x) {
            disks[index].position.x -= 0.05;
        } else {
            return false;
        }
        return true;
}

void resolve_movement() {
    int index = active_disk_index;
    if (!raise_disk()) {
        glutPostRedisplay();
        return;
    };

    u = std::clamp(u + step_u, 0.0, 1.0);
    if (std::abs(disks[index].position.x - dest_pos.x) <= 0.05) {
        disks[index].position.x = dest_pos.x;
    }

    if (move_disk_to_rod()) {
        glutPostRedisplay();
        return;
    }

    if (u >= 1.0 && disks[index].position.z <= dest_pos.z) {
        disks[index].position.z = dest_pos.z;
        is_moving = false;
        u = 0.0;
        disks[active_disk_index].normal = glm::vec3(0, 0, 1);
        active_disk_index = -1;
    }

    if (u == 1.0 && disks[index].position.z > dest_pos.z ) {
        disks[index].normal = glm::vec3(0, 0, 1);
        disks[index].position.z -= 0.05;
    }
    glutPostRedisplay();
}

void next_step() {
    auto [from, to] = solutions.front();
    auto occupied = board.rods[from].occupied;
    solutions.pop_front();
    int i;
    for (i = NUM_DISKS; i >= 0 && occupied[i] < 0; i--);
    int index = occupied[i];

    if (index >= 0) {
        active_disk_index = index;
    }
    move_disk(from, to);
    if (solutions.empty())  {
        do_solve = false;
    }
}

void animation_handler() {
    int curr_time = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = curr_time - prev_time;
    if (elapsed < 1000 / speed_modifier) return;

    prev_time = curr_time;

    if (do_solve && is_moving == false) {
        next_step();
    }
    if (is_moving) {
        resolve_movement();
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Towers of Hanoi");

    glutDisplayFunc(display_handler);
    glutIdleFunc(animation_handler);
    glutKeyboardFunc(keyboard_handler);
    glutReshapeFunc(reshape_handler);

    initialize();

    glutMainLoop();
    return 0;
}
