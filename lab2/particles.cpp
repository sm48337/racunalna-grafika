#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define NUM_PARTICLES 10000
#define LIFETIME 10000
#define BATCH_SIZE 10


int prev_time = 0;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    int life;
};

size_t next_particle = 0;

std::array<Particle, NUM_PARTICLES> particles{};

GLuint particle_texture;

GLuint load_texture(const std::string &texture_path) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    int width, height, nrChannels;

    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    return texture_id;
}

void display_handler() {
    glClear(GL_COLOR_BUFFER_BIT);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particle_texture);
    glEnable(GL_TEXTURE_2D);

    for (const Particle &p : particles) {
        //if (p.life <= 0) continue;
        glPushMatrix();
        glTranslatef(p.position.x, p.position.y, p.position.z);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(0.0, 0.0, 0.0);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(10.0, 0.0, 0.0);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(10.0, 10.0, 0.0);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(0.0, 10.0, 0.0);
        glEnd();
        glPopMatrix();
    }
    glFlush();
    glutSwapBuffers();
}

void generate_particles() {
    for (int i = 0; i < BATCH_SIZE; ++i) {
        Particle &p = particles[next_particle];
        p.position = { std::rand() % 2, std::rand() % 2, 0 };
        p.velocity = { std::rand() % 2, std::rand() % 2, 0 };
        p.life = LIFETIME;
        next_particle = (next_particle + 1) % NUM_PARTICLES;
    }
}

void keyboard_handler(unsigned char key, int, int) {
    switch (key) {
    case 27:
    case 'q':
    case 'Q':
        exit(0);
        break;
    default:
        break;
    };
}

void animation_handler() {
    int curr_time = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = curr_time - prev_time;
    prev_time = curr_time;

    std::for_each(particles.begin(), particles.end(), [elapsed](auto p){
            p.life -= elapsed;
            p.position += float(elapsed) * p.velocity;
        }
    );
    if (particles[next_particle].life <= 0) {
        generate_particles();
    }
}

void initialize() {
    glEnable(GL_TEXTURE_2D);
    particle_texture = load_texture("smoke.bmp");
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Particles");

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    initialize();

    glutDisplayFunc(display_handler);
    glutIdleFunc(animation_handler);
    glutKeyboardFunc(keyboard_handler);
    glutMainLoop();
    return 0;
}
