/*
Author: Akshay Krishnan
Description:
Uses OpenGL to render a chessboard.
The board and some pieces (knight and pawns) can be moved based on user input.
Originally developed for ECE6122 at Georgia Tech.
*/

#include <algorithm>
#include <curses.h>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <GL/glut.h> // Include the GLUT header file
#include <iostream>
#include <math.h>
#include <stdlib.h> // standard definitions
#include <vector>

double qh = 1/sqrt(6);
double qangle = 30.0, qaxisx = 1, qaxisy = -1, qaxisz = 0;

// Camera position
float x = 4.0, y = -5.0, z=10.0;
// light 0 position
float l0_x = 0, l0_y = 0, l0_z = 1;
// light 1 position
float l1_x = 5.0, l1_y = 5.0, l1_z = 8.0;
float deltaMove = 0.0;

// Camera looking direction
float lx = 4.0, ly = 4.0, lz = 0.0; // camera points initially along y-axis
float angle = 0.0; // angle of rotation for the camera direction
float deltaAngle = 0.0; // additional angle change when dragging

// pawn locations
std::vector<int> pawnLocX(16, 0);
std::vector<int> pawnLocY(16, 1);
std::vector<int> knightLocX(4, 0);
std::vector<int> knightLocY(4, 0);
// to check whether a particular location is occupied
std::vector<std::vector<int>> boardOccupancy(8, std::vector<int>(8, 0));

//----------------------------------------------------------------------
// Reshape callback
//
// Window size has been set/changed to w by h pixels. Set the camera
// perspective to 45 degree vertical field of view, a window aspect
// ratio of w/h, a near clipping plane at depth 1, and a far clipping
// plane at depth 100. The viewport is the entire window.
//
//----------------------------------------------------------------------
void changeSize(int w, int h)
{
    float ratio = ((float)w) / ((float)h); // window aspect ratio
    glMatrixMode(GL_PROJECTION); // projection matrix is active
    glLoadIdentity(); // reset the projection
    gluPerspective(45.0, ratio, 0.1, 40.0); // perspective transformation
    glMatrixMode(GL_MODELVIEW); // return to modelview mode
    glViewport(0, 0, w, h); // set viewport (drawing area) to entire window
}

/*
 * function to draw both rooks for one team
 * */
void drawRooks()
{
    glPushMatrix();
        glTranslatef(0.5, 0.5, 0.75/2);
        glutSolidCube(0.75);
        glTranslatef(7, 0.0, 0.0);
        glutSolidCube(0.75);
    glPopMatrix();
}

/*
 * function to draw both bishops for one team
 */
void drawBishops()
{
    glPushMatrix();
        glTranslatef(0.5, 0.5, 0.0);
        glutSolidCone(0.75/2, 1.0, 32, 32);
        glTranslatef(3, 0.0, 0.0);
        glutSolidCone(0.75/2, 1.0, 32, 32);
    glPopMatrix();
}

/*
 * Function to draw a queen
 */
void drawQueen()
{
    glPushMatrix();
        glTranslatef(0.5, 0.5, 0.394948);
        glRotatef(33.97, 1, -1, 0);
        glScalef(0.4, 0.4, 0.4);
        glutSolidTetrahedron();
    glPopMatrix();
}

/*
 * Function to draw a king
 */
void drawKing()
{
    glPushMatrix();
        glTranslatef(0.5, 0.5, 0.5);
        glScalef(0.375, 0.375, 0.5);
        glutSolidOctahedron();
    glPopMatrix();
}

/*
 * Function to draw the pawns based on their saved locations.
 */
void drawPawns()
{
    for(int i =0; i < pawnLocX.size(); i++)
    {
        // white pawns have indices 0-7
        if(i < 8)
        {
            glColor3ub(140, 140, 135);
        }
        else
        {
            glColor3ub(150, 75, 0);
        }
        if(pawnLocX[i] >= 0 && pawnLocX[i] <8)
        {
            glPushMatrix();
            glTranslatef(pawnLocX[i], pawnLocY[i], 0);
            glTranslatef(0.5, 0.5, 0.375);
            glutSolidSphere(0.375, 32, 32);
            glPopMatrix();
        }
    }
}

/*
 * Function to draw the knights based on their locations
 */
void drawKnights()
{
    for(int i =0; i < knightLocX.size(); i++)
    {
        // white knights
        if(i < 2)
        {
            glColor3ub(140, 140, 135);
        }
        else
        {
            glColor3ub(150, 75, 0);
        }
        if(knightLocX[i] >= 0 && knightLocX[i] <8)
        {
            glPushMatrix();
                glTranslatef(knightLocX[i], knightLocY[i], 0);
                glTranslatef(0.5, 0.5, 0.374148);
                glScalef(0.75/3, 0.75/3, 1.0/2);
                glRotatef(90, 1, 0, 0);
                glRotatef(90, 0, 1, 0);
                glutSolidTeapot(1);
            glPopMatrix();
        }
    }
}

/*
 * function that draws all the pieces by calling their respective functions.
 */
void drawPieces()
{
    drawPawns();
    drawKnights();

    // Draw white pieces
    glColor3ub(140, 140, 135); // set drawing color to white
    glPushMatrix();
        drawRooks();
        glTranslatef(2, 0, 0);
        drawBishops();
        glTranslatef(1, 0, 0);
        drawQueen();
        glTranslatef(1, 0, 0);
        drawKing();
    glPopMatrix();
    glColor3ub(150, 75, 0); // set drawing color to white
    glPushMatrix();
        glTranslatef(0, 7, 0);
        drawRooks();
        glTranslatef(2, 0, 0);
        drawBishops();
        glTranslatef(1, 0, 0);
        drawQueen();
        glTranslatef(1, 0, 0);
        drawKing();
    glPopMatrix();
}


/*
 * Updating camera pose if it has been moved.
 */
void updateCameraPose()
{
    double x1 = cos(deltaAngle)*(x - 4) - sin(deltaAngle)*(y-4);
    double y1 = cos(deltaAngle)*(y-4) + sin(deltaAngle)*(x-4);
    double l0_x1 = cos(deltaAngle)*(l0_x - 4) - sin(deltaAngle)*(l0_y-4);
    double l1_x1 = cos(deltaAngle)*(l1_x - 4) - sin(deltaAngle)*(l1_y-4);
    double l0_y1 = cos(deltaAngle)*(l0_y-4) + sin(deltaAngle)*(l0_x-4);
    double l1_y1 = cos(deltaAngle)*(l1_y-4) + sin(deltaAngle)*(l1_x-4);

    x = x1+4;
    y = y1+4;
    l1_x = l1_x1 + 4;
    l0_x = l0_x1 + 4;
    l1_y = l1_y1 + 4;
    l0_y = l0_y1 + 4;
    deltaAngle = 0.0;
}

/*
 * Funtion to draw the entire scene
    We first update the camera location based on its distance from the
    origin and its direction.
 */
void renderScene(void)
{
    int i, j;

    // Clear color and depth buffers
    glClearColor(0.0, 0.0, 0.0, 1.0); // sky color is light blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformations
    glLoadIdentity();

    // Set the camera centered at (x,y,z) and looking along directional
    // vector (lx, ly, 0), with the z-axis pointing up
    gluLookAt(
            x, y, z,
            lx, ly, lz,
            0.0, 0.0, 1.0);

    // Draw ground - 200x200 square colored green
    for(int i =0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            if((i+j)%2 == 0)
            {
                glColor3f(0.0, 0.0, 0.0);
            }
            else
            {
                glColor3f(1.0, 1.0, 1.0);
            }
            glBegin(GL_QUADS);
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(j, i, 0.0);
            glVertex3f(j+1, i, 0.0);
            glVertex3f(j+1, i+1, 0.0);
            glVertex3f(j, i+1, 0.0);
            glEnd();
        }
    }
    drawPieces();

    GLfloat light1_position[] = {l1_x, l1_y, l1_z, 1.0};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    GLfloat light0_position[] = {l0_x, l0_y, l0_z, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glutSwapBuffers(); // Make it all visible
}

/* Function to move a random pawn forward by one step */
void movePawn()
{
    std::vector<bool> tried(16, false);
    bool moved = false;
    while(!std::all_of(tried.begin(), tried.end(), [](int i){return i == true;})
    && !moved)
    {
        int id = rand() % 16;
        tried[id] = true;
        if((id < 8 && boardOccupancy[pawnLocY[id]+1][pawnLocX[id]] != 0) ||
                (id >= 8 && boardOccupancy[pawnLocY[id]-1][pawnLocX[id]] != 0))
        {
            continue;
        }
        else
        {
            if(id < 8)
            {
                boardOccupancy[pawnLocY[id]+1][pawnLocX[id]] = 1;
                boardOccupancy[pawnLocY[id]][pawnLocX[id]] = 0;
                ++pawnLocY[id];
            }
            else
            {
                boardOccupancy[pawnLocY[id]-1][pawnLocX[id]] = 1;
                boardOccupancy[pawnLocY[id]][pawnLocX[id]] = 0;
                --pawnLocY[id];
            }
            moved = true;
        }
    }

}

/*
 * Function to move a random knight in a knight-move.
 */
void moveKnight()
{
    bool moved = false;
    while(!moved)
    {
        int id = rand() % 4;
        for(int i = -1; i < 2 && !moved; i+=2)
        {
            for(int j = -2; j < 3 && !moved; j+=4)
            {
                if(knightLocX[id]+i > 0 && knightLocX[id]+i < 8 &&
                        knightLocY[id]+j > 0 && knightLocY[id]+j < 8
                        && boardOccupancy[knightLocY[id]+j][knightLocX[id]+i] == 0)
                {
                    boardOccupancy[knightLocY[id]+j][knightLocX[id]+i] = 1;
                    boardOccupancy[knightLocY[id]][knightLocX[id]] = 0;
                    knightLocX[id] = knightLocX[id] + i;
                    knightLocY[id] = knightLocY[id] + j;
                    moved = true;
                }
            }
        }
        for(int i = -2; i < 3 && !moved; i+=4)
        {
            for(int j = -1; j < 2 && !moved; j+=2)
            {
                if(knightLocX[id]+i > 0 && knightLocX[id]+i < 8 &&
                   knightLocY[id]+j > 0 && knightLocY[id]+j < 8
                   && boardOccupancy[knightLocY[id]+j][knightLocX[id]+i] == 0)
                {
                    boardOccupancy[knightLocY[id]+j][knightLocX[id]+i] = 1;
                    boardOccupancy[knightLocY[id]][knightLocX[id]] = 0;
                    knightLocX[id] = knightLocX[id] + i;
                    knightLocY[id] = knightLocY[id] + j;
                    moved = true;
                }
            }
        }
    }
}


/*
 * User-input keyboard callbacks.
 * This callback functions calls different functions based on user input.
 */
void processNormalKeys(unsigned char key, int xx, int yy)
{
    switch(key)
    {
        case 'h':
            qh += 0.0001;
            std::cout << "qh " << qh << std::endl;
            break;
        case 'j':
            qh -= 0.0001;
            std::cout << "qh " << qh << std::endl;
            break;
        case 'a':
            qangle+= 0.01;
            std::cout << "qangle " << qangle << std::endl;
            break;
        case 'b':
            qangle -= 0.01;
            std::cout << "qangle " << qangle << std::endl;
            break;
        case 'd':
        case 'D':
            z -= 0.25;
            break;
        case 'u':
        case 'U':
            z += 0.25;
            break;
        case '0':
            if(glIsEnabled(GL_LIGHT0))
            {
                glDisable(GL_LIGHT0);
            }
            else
            {
                glEnable(GL_LIGHT0);
            }
            break;
        case '1':
            if(glIsEnabled(GL_LIGHT1))
            {
                glDisable(GL_LIGHT1);
            }
            else
            {
                glEnable(GL_LIGHT1);
            }
            break;
        case 'r':
        case 'R':
            deltaAngle = -10.0/360*M_PI;
            updateCameraPose();
            break;
        case 'p':
        case 'P':
            movePawn();
            break;
        case 'k':
        case 'K':
            moveKnight();
            break;
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay(); // redisplay everything
}


/*
 * Initialization before callbacks are assigned.
 */
void init()
{
    // state of the board
    for(int i =0; i < 16; i++)
    {
        if(i < 8)
        {
            boardOccupancy[0][i] = 1;
            boardOccupancy[1][i] = 1;
            pawnLocY[i] = 1;
        }
        else
        {
            boardOccupancy[6][i-8] = 1;
            boardOccupancy[7][i-8] = 1;
            pawnLocY[i] = 6;
        }
        pawnLocX[i] = i%8;
    }

    // positions of knights and pawns
    for(int i =0; i < 4; i++)
    {
        if(i < 2)
        {
            knightLocY[i] = 0;
        }
        else
        {
            knightLocY[i] = 7;
        }
        if(i % 2 == 0)
        {
            knightLocX[i] = 1;
        }
        else
        {
            knightLocX[i] = 6;
        }
    }
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);   // enabled smoothness

    // light 0 only has an ambient component
    GLfloat light0_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat light0_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light0_specular[] = { 0.0, 0.0, 0.0, 1.0 };

    // light 1 only has diffuse and specular components
    GLfloat light1_diffuse[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat light1_specular[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat light1_ambient[] = {0.0, 0.0, 0.0, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    GLfloat light1_position[] = {l1_x, l1_y, l1_z, 1.0};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    // using the default values for light0_position
    GLfloat light0_position[] = {l0_x, l0_y, l0_z, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_DEPTH_TEST); // enabled depth test

    srand(time(0));
}

/*
 * Main function to initialize and register callbacks.
 */
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Chess");
    init();

    // register callbacks
    glutReshapeFunc(changeSize); // window reshape callback
    glutDisplayFunc(renderScene); // (re)display callback
    glutKeyboardFunc(processNormalKeys); // process key inputs

    // enter GLUT event processing cycle
    glutMainLoop();

    return 0;
}
