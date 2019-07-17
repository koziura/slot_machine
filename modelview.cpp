#include "modelview.h"
#include "system.h"
#include "bmpimage.h"
#include "tgaimage.h"

#	include <GL/gl.h>
#ifndef GL_GLEXT_VERSION
#	include "glext.h"
#else
#	include <GL/glext.h>
#endif

#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory.h>
#include <time.h>

#ifndef GL_MULTISAMPLE
#	define GL_MULTISAMPLE  0x809D
#endif

static GLfloat lightPosition[4] = { 0.f, 0.f, 1.f, 1.f };

ModelView* ModelView::MV;

#ifdef WIN32
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

void gettimeofday(struct timeval* tp, struct timezone* /*tzp*/)
{
	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
}
#endif

static double ftime(void) {
	struct timeval t;
	gettimeofday(&t, NULL);

	return 1.0*t.tv_sec + 1e-6*t.tv_usec;
}

static double last_T;

void RenderString(float x, float y, void *font, const char* string, RGB_t const& rgb)
{
	const unsigned char* c((unsigned char*)string);

	glColor3f(rgb.r, rgb.g, rgb.b);
	glRasterPos2f(x, y);

	glutBitmapString(font, c);
}

bool isExtensionSupported(const char* ext)
{
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	const char* start      = extensions;
	const char* ptr;

	if (!extensions) {
		return false;
	}

	while ((ptr = strstr(start, ext)) != NULL) {
		// we've found, ensure name is exactly ext
		const char * end = ptr + strlen ( ext );

		if (isspace(*end) || *end == '\0') {
			return true;
		}

		start = end;
	}

	return false;
}

ModelView::ModelView(int* pargc, char** argv) : m_perspective(true),
	m_button_hover(false), m_blick_btn(0), m_lets_game(false), m_winner(false)
{
	MV = this;

	glutInit(pargc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA) ;

	int width = 640;
	int height = 480;

	m_ortho_sz.x = 2;
	m_ortho_sz.y = 2;
	m_ortho_sz.z = 2;

	m_angl_stop_1 = 53;
	m_angl_stop_2 = 180;
	m_angl_stop_3 = 297;

	glutInitWindowSize(width, height);

	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-width) / 2,
						   (glutGet(GLUT_SCREEN_HEIGHT)-height) / 2);

	glutCreateWindow("Slot Machine");
	glutDisplayFunc(&ModelView::paintGL);
	glutIdleFunc(&ModelView::idleGL);
	glutReshapeFunc(&ModelView::resizeGL);

	glutMouseFunc(&ModelView::mouseClick);
	glutMotionFunc(&ModelView::mouseMove);
	glutPassiveMotionFunc(&ModelView::mouseMove);

	glutKeyboardFunc(KeyPress);

//	if ( !isExtensionSupported("GL_ARB_multitexture")) {
//		printf ( "ARB_multitexture NOT supported.\n" );
//	}

	memset(&MV->m_fps_str[0], 0, sizeof(MV->m_fps_str));

	const char * vendor    = (const char *)glGetString ( GL_VENDOR     );
	const char * renderer  = (const char *)glGetString ( GL_RENDERER   );
	const char * version   = (const char *)glGetString ( GL_VERSION    );
	//const char * extension = (const char *)glGetString ( GL_EXTENSIONS );

	printf ( "Vendor:   %s\nRenderer: %s\nVersion:  %s\n", vendor, renderer, version );

	initializeGL();
}

ModelView::~ModelView()
{
	//I use glutDestroyWindow(int handle);

	//glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_CONTINUE_EXECUTION);

	glutLeaveMainLoop();

	glutExit();
}

void ModelView::animate(double speed)
{
	for (int i(0); i < cyl_num; ++i) {
		accelerator[i] += float(speed / factor) * 2.5f;
	}

	glutPostRedisplay();
}

void ModelView::idleGL()
{
	if (!MV->m_lets_game) {
		glutPostRedisplay();
		return;
	}

	const double now_T = ftime();
	const double delta_T = now_T - last_T;
	last_T = now_T;

	const double speed = delta_T * 60;

	MV->animate(speed);

	for (int i(0); i < cyl_num; ++i) {
		auto& angle = MV->angle[i];

		if (!MV->m_stopped[i]) {
			angle += MV->speed[i] + MV->accelerator[i];

			if (angle > 359.f) {
				angle = 0.f;
			}
		}
	}

	glutPostRedisplay();
}

void ModelView::loop()
{
	glutMainLoop();
}

void ModelView::applyOrthoGL(int w, int h)
{
	if (w == -1) {
		w = 1;
	}

	if (h == -1) {
		h = 1;
	}

	m_height = h, m_width = w;

	float ohw(m_ortho_sz.y * h / w);

	if (m_perspective) {
		gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 1, m_ortho_sz.x * 10);
	} else {
		glOrtho(-m_ortho_sz.x, m_ortho_sz.x, -ohw, ohw, -m_ortho_sz.z, m_ortho_sz.z);
	}
}

void ModelView::resizeGL(int width, int height)
{
#ifdef _WIN32
	HWND hwnd_glut = FindWindow(NULL, tx("Slot Machine"));

	if (hwnd_glut != NULL && hwnd_glut != INVALID_HANDLE_VALUE) {
		long dwStyle = GetWindowLong(hwnd_glut, GWL_STYLE);
		dwStyle ^= WS_MAXIMIZEBOX | WS_THICKFRAME;
		SetWindowLong(hwnd_glut, GWL_STYLE, dwStyle);
	}
#endif

	//width = 640;
	//height = 480;

	//glutInitWindowSize(width, height);
	//glutReshapeWindow(width, height);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	MV->applyOrthoGL(width, height);

	glMatrixMode(GL_MODELVIEW);
}

void ModelView::initializeGL()
{
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);

	GLfloat LightModelAmbient[] = { 0.025f, 0.025f, 0.025f, 1.f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

	GLfloat m_shininess = 120;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_shininess);

	//GLfloat MateriaSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	GLfloat MateriaSpecular[] = { 0.32f, 0.34f, 0.43f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, MateriaSpecular);

	GLfloat LightAmbient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);

	GLfloat LightDiffuse[] = { 0.75f, 0.75f, 0.75f, 10.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);

	GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 10.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

	GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

	GLfloat LightAmbient2[] = { 0.35f, 0.35f, 0.35f, 1.0f };

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient2);

	GLfloat LightDiffuse2[] = { 0.75f, 0.75f, 0.75f, 10.0f };

	glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse2);

	int	maxTextureUnits;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);

	printf("ARB_multitexture supported.\nMax texture units %d.\n",
		   maxTextureUnits);

	loadTextures();

	//glClearColor(1.f, 1.f, 1.f, 1.f);

	for (int i(0); i < cyl_num; ++i) {
		cylinder[i] = gluNewQuadric();
		gluQuadricTexture(cylinder[i], GL_TRUE);
		angle[i] = m_angl_stop_2;
	}

	timerTick(0);
}

void ModelView::loadTextures()
{
	char buff[100];
	std::string filename;

	glGenTextures(8, &texture[0]);

	int tex_id = 0;

	for (int i(0); i < 2; ++i) {
		memset(buff, 0, sizeof(buff));
		snprintf(buff, sizeof(buff), "%s%02d.bmp", "tx_bg", i+1);

		filename = buff;

		BmpImage im;
		System::instance().loadBmpImage(filename.c_str(), &im);

		glBindTexture(GL_TEXTURE_2D, texture[tex_id]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, im.width, im.height, 0, GL_BGR, GL_UNSIGNED_BYTE, im.bits());

		++tex_id;
	}

	for (int i(0); i < cyl_num; ++i) {
		memset(buff, 0, sizeof(buff));
		snprintf(buff, sizeof(buff), "%s%02d.tga", "tx_X", i+1);

		filename = buff;

		TgaImage im;
		System::instance().loadTgaImage(filename.c_str(), &im);

		glBindTexture(GL_TEXTURE_2D, texture[tex_id]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.width, im.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, im.bits());
		++tex_id;
	}

	{
		filename = "author.tga";

		TgaImage im;
		System::instance().loadTgaImage(filename.c_str(), &im);

		glBindTexture(GL_TEXTURE_2D, texture[tex_id]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.width, im.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, im.bits());
		++tex_id;
	}


}

void ModelView::timerTick(int /*value*/)
{
	static const int timer_ms = 250;

	if (MV->m_lets_game) {
		int processed_quantity = 0;
		int time_stoped = timer_ms * 20;
		int queue_ms = 1000;

		for (int i(cyl_num-1); i >= 0; i--) {
			auto& stopped = MV->m_stopped[i];
			auto& passed = MV->m_passed[i];

			if (!stopped && passed <= time_stoped + (((i-cyl_num) * -1) * queue_ms)) {
				passed += timer_ms;

				++processed_quantity;
			} else {
				stopped = true;
				passed = 0;

				auto& angle = MV->angle[i];

				int op = 1;

				// closer
				while (angle != MV->m_angl_stop_1 &&
					   angle != MV->m_angl_stop_2 &&
					   angle != MV->m_angl_stop_3)
				{
					angle = (float)(int)angle;

					if (angle < MV->m_angl_stop_2 - 63 && angle > MV->m_angl_stop_1) {
						op = -1;
					} else if (angle < MV->m_angl_stop_2 + 59 && angle > MV->m_angl_stop_2) {
						op = -1;
					} else if (angle < 360 && angle > MV->m_angl_stop_3) {
						op = -1;
					} else {
						op = 1;
					}

					angle = angle + op;

					if (angle >= 360) {
						angle = 0;
					}
				}
			}
		}

		if (!processed_quantity) {
			MV->m_lets_game = false;

			int winner = 0;
			for (int i(0); i < cyl_num; ++i) {
				if (i+1 < cyl_num) {
					winner += MV->angle[i] == MV->angle[i+1];
				} else {
					winner += MV->angle[i-1] == MV->angle[i];
				}
			}

			if (winner == cyl_num) {
				MV->m_winner = true;
				printf("YOU ARE WINNER!!!\n");
			} else {
				printf("GAME OVER\n");
			}
		}
	}

	++MV->m_blick_btn %= 2;

	glutTimerFunc(timer_ms, &ModelView::timerTick, 0);
}

void ModelView::drawCylinder(int idx)
{
	glPushMatrix(); {
		glEnable(GL_TEXTURE_2D);

		glEnable(GL_LIGHTING);

		glRotatef(-60, 0.0, 1.0, 0.0);
		glRotatef(-MV->angle[idx], 0.0, 0.0, 1.0);

		float width = 0.3f;

		glTranslatef(0.f, 0.f, -1.5);

		glTranslatef(0.f, 0.f, idx / 1.95f);

		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		//glDepthMask(GL_FALSE);

		//glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);

		glBindTexture(GL_TEXTURE_2D, texture[idx%2]);

		gluCylinder(MV->cylinder[idx], width, width, 0.5f, 32, 32);

		glEnable(GL_BLEND);
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);

		glBindTexture(GL_TEXTURE_2D, texture[(cyl_num+1) - idx]);
		gluCylinder(MV->cylinder[idx], width, width, 0.5f, 32, 32);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);

		//glDepthFunc(GL_LEQUAL);
	} glPopMatrix();
}

void ModelView::drawFrame()
{
	glPushMatrix(); {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);

		glRotatef(25, 0.0, 1.0, 0.0);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glBindTexture(GL_TEXTURE_2D, texture[1]);

		glBegin(GL_QUADS); {
			glTexCoord3f(0, 0, 0);
			glVertex3f(-1.04, 0.48, 0.5);
			glTexCoord3f(1, 0, 0);
			glVertex3f(1.1, 0.48, 0.5);
			glTexCoord3f(1, 1, 0);
			glVertex3f(1.1, 0.2, 0.5);
			glTexCoord3f(0, 1, 0);
			glVertex3f(-1.04, 0.2, 0.5);

			glTexCoord3f(1, 0, 0);
			glVertex3f(-1.04, -0.48, 0.5);
			glTexCoord3f(1, 1, 0);
			glVertex3f(1.1, -0.48, 0.5);
			glTexCoord3f(0, 1, 0);
			glVertex3f(1.1, -0.2, 0.5);
			glTexCoord3f(0, 0, 0);
			glVertex3f(-1.04, -0.2, 0.5);
		} glEnd();

		glEnable(GL_BLEND);
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);

		//glDisable(GL_LIGHTING);
		//glEnable(GL_LIGHT0);
		glBindTexture(GL_TEXTURE_2D, texture[7]);

		glBegin(GL_QUADS); {
			glTexCoord3f(0, 1, 0);
			glVertex3f(-1.04, 0.48, 0.5);

			glTexCoord3f(1, 1, 0);
			glVertex3f(1.1, 0.48, 0.5);

			glTexCoord3f(1, 0, 0);
			glVertex3f(1.1, 0.2, 0.5);

			glTexCoord3f(0, 0, 0);
			glVertex3f(-1.04, 0.2, 0.5);
		} glEnd();

		glDisable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);

		glDisable(GL_LIGHT0);

		glRotatef(-85, 0.0, 1.0, 0.0);
		glTranslatef(-0.7f, -0.5f, 1.06);
		glBindTexture(GL_TEXTURE_2D, texture[0]);

		glBegin(GL_QUADS); {
			glTexCoord3f(0, 1, 0);
			glVertex3f(-1, 0, 0);
			glTexCoord3f(0, 0, 0);
			glVertex3f(1, 0, 0);
			glTexCoord3f(1, 0, 0);
			glVertex3f(1, 1, 0);
			glTexCoord3f(1, 1, 0);
			glVertex3f(-1, 1, 0);
		} glEnd();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);

		glDepthFunc(GL_LEQUAL);
	} glPopMatrix();
}

void ModelView::drawButton()
{
	glPushMatrix(); {
		Vec2 l_top, r_top;
		Vec2 l_bottom, r_bottom;

		float size = 0.25f;

		r_top.x = size*2;
		r_bottom.x = size*2;
		r_bottom.y = size;
		l_bottom.y = size;

		glTranslatef(-1.f, -1.f, 0);

		if (m_button_hover) {
			glColor3f(1.0f, 0.0f, 0.f);
		} else {
			switch (m_blick_btn) {
			case 0:
				glColor3f(0.0f, 0.0f, 1.f);
			break;
			case 1:
				glColor3f(1.0f, 0.0f, 1.f);
			break;
			default:
			break;
			}
		}

		glBegin(GL_QUADS); {
			glVertex2d(l_top.x,	l_top.y);
			glVertex2d(r_top.x, r_top.y);
			glVertex2d(r_bottom.x, r_bottom.y);
			glVertex2d(l_bottom.x, l_bottom.y);
		} glEnd();

		RenderString(0.050f, 0.09f, GLUT_BITMAP_TIMES_ROMAN_24, "START",
					 RGB_t(1.0f, 1.0f, 0.f));
	} glPopMatrix();
}

void ModelView::paintGL()
{
//	GLint srcBled;
//	glGetIntegerv(GL_BLEND_SRC, &srcBled);

//	GLint dstBled;
//	glGetIntegerv(GL_BLEND_DST, &dstBled);

	MV->frame++;
	MV->time = glutGet(GLUT_ELAPSED_TIME);

	if (MV->time - MV->timebase > 1000) {
		memset(&MV->m_fps_str[0], 0, sizeof(MV->m_fps_str));

		sprintf(MV->m_fps_str,"FPS:%4.2f",
				MV->frame * 1000.0 / (MV->time-MV->timebase));

		MV->timebase = MV->time;
		MV->frame = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	if (MV->m_perspective) {
		gluLookAt
		(
			0, 0, MV->m_ortho_sz.x * 1.5,
			0, 0, 0,
			0, 1, 0
		);
	}

	for (int i(0); i < cyl_num; ++i) {
		MV->drawCylinder(i);
	}

	MV->drawFrame();

	glDisable(GL_LIGHTING);

	if (!MV->m_lets_game) {
		MV->drawButton();
	}

	if (MV->m_winner) {
		RenderString(-0.5f, -0.5f, GLUT_BITMAP_TIMES_ROMAN_24, "!!!YOU ARE WINNER!!!",
					 RGB_t(1.0f, 1.0f, 0.0f));
	}

	RenderString(1.f, -1.f, GLUT_BITMAP_TIMES_ROMAN_24, (char*)&MV->m_fps_str,
				 RGB_t(1.0f, 1.0f, 1.0f));

	//glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	//glBlendFunc( srcBled, dstBled );

	/*  Flush and swap */
	glFlush();
	glutSwapBuffers();
}

void ModelView::mouseClick(int button, int state, int /*x*/, int /*y*/)
{
	if (MV->m_button_hover && !MV->m_lets_game &&
			button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		printf("Start button clicked\n");

		for (int i(0); i < cyl_num; ++i) {
			MV->m_stopped[i] = false;
			MV->m_passed[i] = 0;
			MV->accelerator[i] = 0;

			//speed[i] = static_cast <float> (rand()) / static_cast<float>(RAND_MAX);
			MV->speed[i] = dbl_rand(0.1f, 10.f);

			MV->m_passed[i] = 0;
		}

		int min = 20;
		int max = 200;
		MV->factor = min + (rand() % (int)(max - min + 1));

		printf("speed factor: %d\n", MV->factor);

		MV->m_winner = false;
		MV->m_lets_game = true;

		last_T = ftime();
	}
}

void ModelView::mouseMove(int x, int y)
{
	float mouseX = ((float)x / MV->m_width) - 0.5f;
	float mouseY = ((float)y / MV->m_height) - 0.5f;

	if ((mouseX >= -0.3 && mouseX <= -0.15) && (mouseY >= 0.3 && mouseY <= 0.4)) {
		MV->m_button_hover = true;
	} else {
		MV->m_button_hover = false;
	}
}

void ModelView::KeyPress(unsigned char key, int /*x*/, int /*y*/)
{
	using namespace std;

	switch (key)
	{
		case '1':
			MV->angle[0] +=1.f;

			if (MV->angle[0] >= 360) {
				MV->angle[0] = 0;
			}

			cout << MV->angle[0] << endl;
			break;
		case '2':
			MV->angle[0] -=1.f;

			if (MV->angle[0] < 0) {
				MV->angle[0] = 359;
			}

			cout << MV->angle[0] << endl;
			break;
		case '3':
			MV->angle[4] +=1.f;

			if (MV->angle[4] >= 360) {
				MV->angle[4] = 0;
			}

			cout << MV->angle[4] << endl;
			break;
		case '4':
			MV->angle[4] -=1.f;

			if (MV->angle[4] < 0) {
				MV->angle[4] = 359;
			}

			cout << MV->angle[4] << endl;
			break;
	}

	glutPostRedisplay();
}
