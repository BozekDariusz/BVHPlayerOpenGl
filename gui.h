#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QOpenGLWindow>
#include <QSurfaceFormat>
#include <QOpenGLFunctions>
#include <QtOpenGL>
//#include <gl/GL.h>
#include <gl/GLU.h>
#include <QFileDialog>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include<cmath>
#include <math.h>
#include <Windows.h>
#include "ui_gui.h"
QT_BEGIN_NAMESPACE
namespace Ui { class GUI; }
QT_END_NAMESPACE


#define CAMERASPEED	0.03f
typedef struct tVector3					// expanded 3D vector struct
{
	tVector3() {}	// constructor
	tVector3(float new_x, float new_y, float new_z) // initialize constructor
	{
		x = new_x; y = new_y; z = new_z;
	}
	// overload + operator so that we easier can add vectors
	tVector3 operator+(tVector3 vVector) { return tVector3(vVector.x + x, vVector.y + y, vVector.z + z); }
	// overload - operator that we easier can subtract vectors
	tVector3 operator-(tVector3 vVector) { return tVector3(x - vVector.x, y - vVector.y, z - vVector.z); }
	// overload * operator that we easier can multiply by scalars
	tVector3 operator*(float number) { return tVector3(x*number, y*number, z*number); }
	// overload / operator that we easier can divide by a scalar
	tVector3 operator/(float number) { return tVector3(x / number, y / number, z / number); }
	void norm() {
		float mag = sqrt (pow(x, 2) + pow(y, 2) + pow(z, 2));
		x = x / mag;
		y = y / mag;
		z = z / mag;
	}

	float x, y, z;						// 3D vector coordinates
}tVector3;

struct root {
	bool isroot=false;
	std::string name;
	tVector3 offset;
	std::vector<root*> children;
	root *parent;
	std::vector<std::string> channels;
	bool isEnd = false;
	tVector3 endOffset;
//	glm::mat4 m4;
	tVector3 IKOffset{0,0,0};
	tVector3 position;
};

class animationWindow : public QOpenGLWindow
{
    Q_OBJECT

	

public:

	root *skeleton;
	int currentFrame = 0;
	int *currentFramePoint = &currentFrame;
	int currentJointValue = 0;
	int *currentJointPoint = &currentJointValue;
	animationWindow(QWidget *parent = 0);
    ~animationWindow();
	void Keyboard_Input();
	bool look;
	int y;
	std::vector<root> toDraw;
	std::vector<std::vector<std::vector<float>>> frames;
	QTimer *timer;
	bool play=true;
	bool animation=true;


protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
	void drawJointsByOffset(root *joint, int *currentFrame, int *currentJoint);
	void drawJointsByPossition(root* joint);
    void resizeEvent(QResizeEvent *event);
	//void animationWindow::getIdentityMatrix(root *joint, std::vector<float> frames);

public slots:
        void UpdateAnimation();
		void paintBVHEvent(root* model, std::vector<std::vector<std::vector<float>>>);

      

private:
    QOpenGLContext *context;
    QOpenGLFunctions *openGLFunctions;

};
class CCamera
{
	public:

		tVector3 mPos;
		tVector3 mView;
		tVector3 mUp;

		//NEW//////////////////NEW//////////////////NEW//////////////////NEW////////////////
				// This function let you control the camera with the mouse
				void Mouse_Move(int wndWidth, int wndHeight);

				//NEW//////////////////NEW//////////////////NEW//////////////////NEW////////////////
						void Move_Camera(float speed);
						void Rotate_View(float speed);
						void Position_Camera(float pos_x, float pos_y,float pos_z,
											 float view_x, float view_y, float view_z,
											 float up_x,   float up_y,   float up_z);
						void Strafe_Camera(float speed);
};





Q_DECLARE_METATYPE(root*);

class GUI : public QMainWindow
{
    Q_OBJECT

		animationWindow *renderWindow;
	root *skeleton;
	root *currentJoint;

	std::vector<std::vector<std::vector<float>>> frames;


public:
    GUI(QWidget *parent = nullptr);
    ~GUI();
	float frameTime;
	void createModelTree();

	QStandardItem* addJointToTree(QStandardItemModel *model, root *joint);
	void addRootToTree(QStandardItemModel *model, root *joint);
	/*
	void  GUI::calculatePositon(root *joint);*/
	void  GUI::calculateRotation(root *joint);
	void FABRIK(root *joint, float changeX, float changeY, float changeZ);

private slots:
    void on_actionLoad_BVH_triggered();

	void setFPS(float * frameTime);
    void on_actionSave_BVH_triggered();
	std::vector<std::vector<std::vector<float>>> loadFrames(std::istream& faceFile);
    void on_treeView_clicked(const QModelIndex &index);

    void on_xSlider_sliderMoved(int position);

    void on_ySlider_sliderMoved(int position);

    void on_zSlider_sliderMoved(int position);

    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_nextButton_clicked();

    void on_prevButton_clicked();

    void on_xSlider_valueChanged(int value);

    void on_translationMode_toggled(bool checked);

    void on_rotationMode_toggled(bool checked);

private:
    Ui::GUI *ui;
};
#endif // GUI_H
