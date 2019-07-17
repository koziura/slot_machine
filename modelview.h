#ifndef MODELVIEW_H
#define MODELVIEW_H

struct RGB_t {
	RGB_t(float r_ = 1.f, float g_ = 1.f, float b_ = 1.f) :
		r(r_), g(g_), b(b_)
	{
	}

	float r,g,b;
};

struct Vec3 {
	Vec3(float x_ = 0.f, float y_ = 0.f, float z_ = 0.f):
		x(x_), y(y_), z(z_)
	{
	}

	float x,y,z;
};

struct Vec2 {
	Vec2(float x_ = 0.f, float y_ = 0.f):
		x(x_), y(y_)
	{
	}

	float x,y;
};

class GLUquadric;
typedef GLUquadric GLUquadricObj;

class ModelView
{
public:
	ModelView(int* pargc, char** argv);
	~ModelView();

	void loop();

private:
	static ModelView* MV;

	bool m_perspective;
	bool m_button_hover;

	int m_blick_btn;

	int m_width, m_height;

	Vec3 m_ortho_sz;
	static const uint8_t cyl_num = 5;

	GLUquadricObj* cylinder[cyl_num];
	float angle[cyl_num];
	float speed[cyl_num];

	float accelerator[cyl_num];
	bool m_stopped[cyl_num];
	int m_passed[cyl_num];

	bool m_lets_game, m_winner;

	unsigned int texture[cyl_num + 3];

	int factor = 200; // factor the animation is slowed down by
	int frame = 0, time, timebase = 0;
	char m_fps_str[16];

	float m_angl_stop_1, m_angl_stop_2, m_angl_stop_3;

private:
	static void resizeGL(int width, int height);
	static void paintGL();
	static void idleGL();
	static void mouseClick(int button, int state, int, int);
	static void mouseMove(int x, int y);
	static void KeyPress(unsigned char key, int, int);
	static void timerTick(int);

	void animate(double speed);

	void initializeGL();
	void applyOrthoGL(int w, int h);

	void loadTextures();

	void drawCylinder(int idx);
	void drawFrame();
	void drawButton();

};


//ModelView::MV = nullptr;

#endif // MODELVIEW_H
