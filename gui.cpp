#include "gui.h"



using namespace std;


const vector<string> explode(const string& s, const char& c)
{
    string buff{ "" };
    vector<string> v;

    for (auto n : s)
    {
        if (n != c) buff += n; else
            if (n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if (buff != "") v.push_back(buff);

    return v;
}

void loadEnd(istream& faceFile,root* joint) {
    string line;
    getline(faceFile, line);
    getline(faceFile, line);
    vector<string> v{ explode(line, ' ') };
	joint->isEnd = true;
	joint->endOffset = (tVector3(stof(v[1]), stof(v[2]), stof(v[3])));
    getline(faceFile, line);
}

root *loadJoint(string name, istream& faceFile,root* parent) {

    string line;
    root *Base=new root;//memory leak 
    Base->parent=parent;
    Base->name = name;
    Base->isroot = false;
    getline(faceFile, line);
    getline(faceFile, line);
    vector<string> v{ explode(line, ' ') };
    Base->offset = (tVector3(stof(v[1]), stof(v[2]), stof(v[3])));
	Base->position = Base->offset + Base->parent->position;
    getline(faceFile, line);
    v = explode(line, ' ');
    for (int i = 2;i < v.size();i++) {
		Base->channels.push_back(v[i]);
    }
    while (v[0].compare("}")) {
        getline(faceFile, line);
        v = explode(line, ' ');
        v[0].erase(std::remove(v[0].begin(), v[0].end(), '\t'), v[0].end());
        if (!v[0].compare("JOINT")) {
            Base->children.push_back(loadJoint(v[1], faceFile,Base));
        }
        if (!v[0].compare("End")) {
           loadEnd( faceFile,Base);
        }
    }
    return Base;
}

root* loadRoot(string name, istream& faceFile) {
    string line;
    root *rootBase=new root;
	rootBase->name = name;
	rootBase->isroot = true;
    getline(faceFile, line);
    getline(faceFile, line);
    vector<string> v{ explode(line, ' ') };
	rootBase->offset = (tVector3(stof(v[1]), stof(v[2]), stof(v[3])));
	rootBase->position = rootBase->offset;
    getline(faceFile, line);
    v = explode(line, ' ');
    for (int i = 2;i < v.size();i++) {
		rootBase->channels.push_back(v[i]);
    }
    while (v[0].compare("}")) {
    getline(faceFile, line);
    v = explode(line, ' ');

    v[0].erase(std::remove(v[0].begin(), v[0].end(), '\t'), v[0].end());
    if (!v[0].compare("JOINT")) {
		rootBase->children.push_back(loadJoint(v[1], faceFile,rootBase));
    }
}
    return rootBase;

}

std::vector<std::vector<std::vector<float>>> GUI::loadFrames(istream& faceFile) {
	string line;
	std::vector<std::vector<std::vector<float>>>Frames;
	getline(faceFile, line);
	vector<string> v{ explode(line, ' ') };
	int frameAmount = stoi(v[1]);
	getline(faceFile, line);
	v = { explode(line, ' ') };
	frameTime = stof(v[2]);
	setFPS(&frameTime);
	for (int i = 0;i < frameAmount;i++) {

		std::vector<std::vector<float>> frame;
		getline(faceFile, line);
		v = { explode(line, ' ') };
		for (int j = 0;j < v.size();j = j + 3) {
			std::vector<float> temp;
			temp.push_back(stof(v[j]));
			temp.push_back(stof(v[j + 1]));
			temp.push_back(stof(v[j + 2]));

			frame.push_back(temp);
		}
		Frames.push_back(frame);
	}
	return Frames;
}

void saveJoint( ostream& BVHFile, root* joint) {

	BVHFile << "JOINT " << joint->name << " \n";
	BVHFile << "{\n";
	BVHFile << "OFFSET " << joint->offset.x << " " << joint->offset.y << " " << joint->offset.z << "\n";
	BVHFile << "CHANNELS " << joint->channels.size() << " ";
	
	for (int i = 0;i < joint->channels.size();i++) {
		BVHFile << joint->channels[i] << " ";
	}
	BVHFile << " \n";
	if (joint->isEnd) {
		BVHFile << "End Site " " \n";
		BVHFile << "{\n";
		BVHFile << "OFFSET " << joint->endOffset.x << " " << joint->endOffset.y << " " << joint->endOffset.z << "\n";
		BVHFile << "} \n";
	}
	for (int i = 0;i < joint->children.size();i++) {
		 saveJoint(BVHFile, joint->children[i]);
	}
	BVHFile << "} \n";


}
void saveFrame(ostream& BVHFile, std::vector<std::vector<float>> frame) {


	for (int i = 0;i < frame.size();i++) {
		for (int j = 0;j < frame[i].size();j++) {

			BVHFile << frame[i][j]<<" "  ;
		}
	}

	BVHFile << "\n ";






}
void drawCylinder(root *child)
{
	float vx = child->offset.x ;
	float vy = child->offset.y ;
	float vz = child->offset.z ;
	if (vz == 0) vz = 0.0001;
	float v = sqrt(vx*vx + vy * vy + vz * vz);
	float ax = 57.2957795 * acos(vz / v);
	if (vz < 0.0) ax = -ax;
	float rx = -vy * vz;
	float ry = vx * vz;
	//draw the cylinder body

	glPushMatrix();
	glRotatef(ax, rx, ry, 0.0);
	GLUquadric *quad;
	quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_FILL);
	gluCylinder(quad, 0.2, 0.2, v, 30, 10);

	glPopMatrix();
}


void drawCylinder2(root *child, root *parent) {



	//(float x1, float y1, float z1, float x2, float y2, float z2)
	
		float vx =  child->position.x- parent->position.x ;
		float vy = child->position.y-parent->position.y ;
		float vz = child->position.z -parent->position.z ;
		if (vz == 0) vz = 0.0001;
		float v = sqrt(vx*vx + vy * vy + vz * vz);
		//radians
		float ax = 57.2957795 * acos(vz / v);
		if (vz < 0.0) ax = -ax;
		float rx = -vy * vz;
		float ry = vx * vz;


		glPushMatrix();
		glRotatef(ax, rx, ry, 0.0);
		GLUquadric *quad;
		quad = gluNewQuadric();
		gluQuadricOrientation(quad, GLU_FILL);
		gluCylinder(quad, 0.2, 0.2, v, 30, 10);
		glPopMatrix();
	






}
GUI::GUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GUI)
{
	 renderWindow = new  animationWindow();
	QWidget *container = QWidget::createWindowContainer(renderWindow);
    container->resize(631,591);
    container->move(266,20);
	this->layout()->addWidget(container);
    ui->setupUi(this);
}

GUI::~GUI()
{
    delete ui;
}

animationWindow::animationWindow(QWidget *parent)
{



	setSurfaceType(QWindow::OpenGLSurface);

	QSurfaceFormat format;
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
	setFormat(format);
	context = new QOpenGLContext;
	context->setFormat(format);
	context->create();
	context->makeCurrent(this);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(UpdateAnimation()));



}

CCamera objCamera;
animationWindow::~animationWindow()
{
}
void animationWindow::initializeGL()
{
	skeleton = new root;
	glEnable(GL_DEPTH_TEST);
	resizeGL(this->width(), this->height());
}

/*
void animationWindow::getIdentityMatrix(root *joint, std::vector<float> frames) {


	int index = 0;
	int temp = currentJointValue;
	joint->m4 = glm::translate(joint->m4, glm::vec3(joint->offset.x,joint->offset.y, joint->offset.z));
	for (int i = 0;i < joint->channels.size();i++) {
		if (i == 3) {
			currentJointValue++;
			index = 0;
		}
		if (!joint->channels[i].compare("Xposition")) {
			joint->m4 = glm::translate(joint->m4, glm::vec3(frames[index], 0.0f, 0.0f));
		}
		if (!joint->channels[i].compare("Yposition")) {

			joint->m4 = glm::translate(joint->m4, glm::vec3(0.0f, frames[index], 0.0f));
		}
		if (!joint->channels[i].compare("Zposition")) {

			joint->m4 = glm::translate(joint->m4, glm::vec3(0.0f, 0.0f, frames[index]));
		}
		if (!joint->channels[i].compare("Zrotation")) {
			joint->m4 = glm::rotate(joint->m4, glm::radians(frames[index]), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (!joint->channels[i].compare("Yrotation")) {

			joint->m4 = glm::rotate(joint->m4, glm::radians(frames[index]), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (!joint->channels[i].compare("Xrotation")) {

			joint->m4 = glm::rotate(joint->m4, glm::radians(frames[index]), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		index++;
	}

	currentJointValue = temp;

	
}*/
void animationWindow::drawJointsByOffset(root* joint,int *tempcurrentFrame,int * tempCurrentJoint) {


	

	glPushMatrix();
	//getIdentityMatrix(joint, frames[*tempcurrentFrame][*tempCurrentJoint]);
	if (!joint->isroot) {
	
		drawCylinder(joint);
	}
	
	//glTranslatef(joint->IKOffset.x, joint->IKOffset.y, joint->IKOffset.z);
	glTranslatef(joint->offset.x, joint->offset.y, joint->offset.z);

	int index = 0;
	for (int i = 0;i < joint->channels.size();i++) {
		if (i == 3) {
			currentJointValue++;
			index = 0;
		}
		if (!joint->channels[i].compare("Xposition")) {
			glTranslatef(frames[*tempcurrentFrame][*tempCurrentJoint][index],0,0);
		}
		if (!joint->channels[i].compare("Yposition")) {

			glTranslatef(0,frames[*tempcurrentFrame][*tempCurrentJoint][index], 0);
		}
		if (!joint->channels[i].compare("Zposition")) {

			glTranslatef(0,0,frames[*tempcurrentFrame][*tempCurrentJoint][index]);
		}
		if (!joint->channels[i].compare("Zrotation")) {
			glRotatef(frames[*tempcurrentFrame][*tempCurrentJoint][index], 0, 0, 1);
		}
		if (!joint->channels[i].compare("Yrotation")) {

			glRotatef(frames[*tempcurrentFrame][*tempCurrentJoint][index], 0, 1, 0);
		}
		if (!joint->channels[i].compare("Xrotation")) {

			glRotatef(frames[*tempcurrentFrame][*tempCurrentJoint][index], 1, 0, 0);
		}
		index++;
	}

	GLUquadric *quad;
	quad = gluNewQuadric();

	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricDrawStyle(quad, GLU_LINE);
	gluSphere(quad, 0.4, 10, 10);

	
	for (int i = 0;i < joint->children.size();i++) {
		currentJointValue++;
		drawJointsByOffset(joint->children[i], tempcurrentFrame, tempCurrentJoint);

		
	}
	glPopMatrix();
}

void animationWindow::drawJointsByPossition(root* joint) {




	glPushMatrix();
	glFlush();

	if (!joint->isroot) {
		glPushMatrix();
		glTranslatef(joint->parent->position.x, joint->parent->position.y, joint->parent->position.z);
		drawCylinder2(joint,joint->parent);
		glPopMatrix();
	}
	glFlush();
	glTranslatef(joint->position.x, joint->position.y, joint->position.z);

	int index = 0;

	GLUquadric *quad;
	quad = gluNewQuadric();

	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricDrawStyle(quad, GLU_LINE);
	gluSphere(quad, 0.4, 10, 10);


	for (int i = 0;i < joint->children.size();i++) {
		currentJointValue++;
		glPopMatrix();
		drawJointsByPossition(joint->children[i]);


	}
	glPopMatrix();
}
void animationWindow::resizeGL(int w, int h)
{
	objCamera.Position_Camera(0, 2.5f, 5, 0, 2.5f, 0, 0, 1, 0);
	glViewport(0, 0, w, h);
	qreal aspectRatio = qreal(w) / qreal(h);

	//initialize projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(75, aspectRatio, 0.1, 400000000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void Draw_Grid()
{

	for (float i = -500; i <= 500; i += 5)
	{
		glBegin(GL_LINES);
		glColor3ub(150, 190, 150);
		glVertex3f(-500, 0, i);
		glVertex3f(500, 0, i);
		glVertex3f(i, 0, -500);
		glVertex3f(i, 0, 500);
		glEnd();
	}
}
void animationWindow::paintGL()
{
	
	Keyboard_Input();
	if (!look) {
		objCamera.Mouse_Move(640, 480);
	}
	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Draw_Grid();
	glLoadIdentity();
	
	
	gluLookAt(objCamera.mPos.x, objCamera.mPos.y, objCamera.mPos.z,
	objCamera.mView.x, objCamera.mView.y, objCamera.mView.z,
		objCamera.mUp.x, objCamera.mUp.y, objCamera.mUp.z);
	//glPushMatrix();
    if (skeleton->isroot) {
        if (!animation) {
			drawJointsByPossition(skeleton);
			currentJointValue = 0;
		}
		else {

				drawJointsByOffset(skeleton, &currentFrame, &currentJointValue);
				currentJointValue = 0;

		
		}
	}
	//glPopMatrix();



	glFlush();
	this->update();
	
}
void animationWindow::resizeEvent(QResizeEvent *event)
{
	resizeGL(this->width(), this->height());

	this->update();
}

void animationWindow::paintBVHEvent(root *model, std::vector<std::vector<std::vector<float>>> GuiFrames)
{
	skeleton = model;
	frames = GuiFrames;
	paintGL();

	this->update();
}

void animationWindow::UpdateAnimation() {
	
	if (play) {
		currentFrame++;
		if (currentFrame >= frames.size()) {
			currentFrame = 0;
		}
	}


}
void GUI::setFPS(float * frameTime)
{
	this->renderWindow->timer->start(1000 * *frameTime);


}

void GUI::on_actionLoad_BVH_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/home",
                                                    tr("BVH ( *.bvh)"));
string current_locale_text = fileName.toLocal8Bit().constData();
      skeleton;
    ifstream faceFile;
    faceFile.open(current_locale_text);

    if (faceFile.is_open()) {

        string line;
        while (getline(faceFile, line)) {
            //cout << line<< endl;
            if (!line.compare("HIERARCHY")) {
                getline(faceFile, line);

                vector<string> v{ explode(line, ' ') };
                if (!v[0].compare("ROOT")) {
                     skeleton = loadRoot(v[1],faceFile);
                }

            }
			if (!line.compare("MOTION")) {
				frames=loadFrames(faceFile);
			}

        }
		renderWindow->paintBVHEvent(skeleton,frames);


        //	
        }
	createModelTree();
	//faceFile.close();
}
void GUI::on_actionSave_BVH_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
		"/home",
		tr("BVH ( *.bvh)"));

	string current_locale_text = fileName.toLocal8Bit().constData();
	ofstream BVHFile;
	BVHFile.open(current_locale_text);
	BVHFile << "HIERARCHY\n";
	if (skeleton) {
		BVHFile << "ROOT " << skeleton->name << " \n";
		BVHFile << "{\n";
		BVHFile << "OFFSET "<<skeleton->offset.x << " " << skeleton->offset.y << " " << skeleton->offset.z <<"\n";
		BVHFile << "CHANNELS " << skeleton->channels.size() << " ";
		for (int i = 0;i < skeleton->channels.size();i++) {
			BVHFile << skeleton->channels[i] << " "  ;
		}
		BVHFile << " \n";
		for (int i = 0;i < skeleton->children.size();i++) {
			 saveJoint(BVHFile, skeleton->children[i]);
		}
		BVHFile << "} \n";
	}
	BVHFile << "MOTION\n";
	BVHFile << "Frames "<<this->renderWindow->frames.size()<<"\n";
	BVHFile << "Frame Time: "<<frameTime<<"\n";
	for (int i = 0;i < renderWindow->frames.size();i++) {
		saveFrame(BVHFile, renderWindow->frames[i]);
	
	}
	BVHFile.close();
}

void CCamera::Position_Camera(float pos_x, float pos_y, float pos_z,
	float view_x, float view_y, float view_z,
	float up_x, float up_y, float up_z)
{
	objCamera.mPos = tVector3(pos_x, pos_y, pos_z); // set position
	objCamera.mView = tVector3(view_x, view_y, view_z); // set view
	objCamera.mUp = tVector3(up_x, up_y, up_z); // set the up vector
}

void CCamera::Rotate_View(float speed)
{
	tVector3 vVector = mView - mPos;	// Get the view vector

	mView.z = (float)(mPos.z + sin(speed)*vVector.x + cos(speed)*vVector.z);
	mView.x = (float)(mPos.x + cos(speed)*vVector.x - sin(speed)*vVector.z);
}

void CCamera::Mouse_Move(int wndWidth, int wndHeight)
{
	POINT mousePos;
	int mid_x = wndWidth >> 1;
	int mid_y = wndHeight >> 1;
	float angle_y = 0.0f;
	float angle_z = 0.0f;

	GetCursorPos(&mousePos);	// Get the 2D mouse cursor (x,y) position

	if ((mousePos.x == mid_x) && (mousePos.y == mid_y)) return;

	SetCursorPos(mid_x, mid_y);	// Set the mouse cursor in the center of the window

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angle_y = (float)((mid_x - mousePos.x)) / 1000;
	angle_z = (float)((mid_y - mousePos.y)) / 1000;

	// The higher the value is the faster the camera looks around.
	objCamera.mView.y += angle_z * 2;

	if ((mView.y - mPos.y) > 8)  mView.y = mPos.y + 8;
	if ((mView.y - mPos.y) < -8)  mView.y = mPos.y - 8;
	Rotate_View(-angle_y); // Rotate
}
void CCamera::Move_Camera(float speed)
{
	tVector3 vVector = mView - mPos;	// Get the view vector

	// forward positive cameraspeed and backward negative -cameraspeed.
	mPos.x = mPos.x + vVector.x * speed;
	mPos.z = mPos.z + vVector.z * speed;
	mView.x = mView.x + vVector.x * speed;
	mView.z = mView.z + vVector.z * speed;
}
void CCamera::Strafe_Camera(float speed)
{
	tVector3 vVector = mView - mPos;	// Get the view vector
	tVector3 vOrthoVector;              // Orthogonal vector for the view vector

	vOrthoVector.x = -vVector.z;
	vOrthoVector.z = vVector.x;

	// left positive cameraspeed and right negative -cameraspeed.
	mPos.x = mPos.x + vOrthoVector.x * speed;
	mPos.z = mPos.z + vOrthoVector.z * speed;
	mView.x = mView.x + vOrthoVector.x * speed;
	mView.z = mView.z + vOrthoVector.z * speed;
}


void animationWindow::Keyboard_Input()//for camera movement
{

	if ((GetKeyState(VK_UP) & 0x80) || (GetKeyState('W') & 0x80))
	{
		objCamera.Move_Camera(CAMERASPEED);
	}

	if ((GetKeyState(VK_DOWN) & 0x80) || (GetKeyState('S') & 0x80))
	{
		objCamera.Move_Camera(-CAMERASPEED);
	}

	if ((GetKeyState(VK_LEFT) & 0x80) || (GetKeyState('A') & 0x80))
	{
		objCamera.Strafe_Camera(-CAMERASPEED);
	}

	if ((GetKeyState(VK_RIGHT) & 0x80) || (GetKeyState('D') & 0x80))
	{
		objCamera.Strafe_Camera(CAMERASPEED);
	}




	if (GetKeyState(0x43) & 0x80 && (y == 0)) {//C
		look = !look;
		y++;
	}
	else if (!(GetKeyState(0x43) & 0x80)) {
		y = 0;

	}
}


void GUI::createModelTree() {

	QStandardItemModel* model = new QStandardItemModel();
	ui->treeView->setModel(model);

	addRootToTree(model, skeleton);


}
void GUI::addRootToTree(QStandardItemModel *model, root *joint) {

	QStandardItem* item = new QStandardItem();
	item->setText(QString::fromStdString(joint->name));
	item->setEditable(false);

	item->setData(QVariant::fromValue(joint));
	QStandardItem* child = new QStandardItem();
	for (int i = 0; i < joint->children.size(); i++) {

		child = addJointToTree(model, joint->children[i]);
		item->setChild(i, child);
	}

	model->appendRow(item);
}
QStandardItem* GUI::addJointToTree(QStandardItemModel *model,root *joint) {
	
	QStandardItem* item = new QStandardItem();
	item->setText(QString::fromStdString(joint->name));
	item->setEditable(false);


	item->setData(QVariant::fromValue(joint));
	QStandardItem* child = new QStandardItem();
	for (int i = 0; i < joint->children.size(); i++) {

		child =addJointToTree(model, joint->children[i]);
		item->setChild(i, child);
	}
	return item;
}

void GUI::on_treeView_clicked(const QModelIndex &index)//todo add rotation labels
{
	
	currentJoint = index.data(Qt::UserRole + 1).value<root*>();
	ui->xSlider->setValue(currentJoint->offset.x*100);
	ui->ySlider->setValue(currentJoint->offset.y * 100);
	ui->zSlider->setValue(currentJoint->offset.z * 100);

	
	if (currentJoint->isroot) {
		if (ui->translationMode->isChecked()) {
			ui->infoLabel->setText(QString::fromStdString("Joint Translation"));
			ui->xLabel->setText(QString::fromStdString(std::to_string(frames[this->renderWindow->currentFrame][0][0])));
			ui->yLabel->setText(QString::fromStdString(std::to_string(frames[this->renderWindow->currentFrame][0][1])));
			ui->zLabel->setText(QString::fromStdString(std::to_string(frames[this->renderWindow->currentFrame][0][2])));
		}
		else {}
	}
	else {
		if (ui->translationMode->isChecked()) {
			ui->infoLabel->setText(QString::fromStdString("Joint Offset"));
			ui->xLabel->setText(QString::fromStdString(std::to_string(currentJoint->offset.x)));
			ui->yLabel->setText(QString::fromStdString(std::to_string(currentJoint->offset.y)));
			ui->zLabel->setText(QString::fromStdString(std::to_string(currentJoint->offset.z)));
		}
		else {}
	}
}

void GUI::on_xSlider_sliderMoved(int position)
{/*
	if (currentJoint) {
	if (ui->translationMode->isChecked()) {
		FABRIK(currentJoint, position/10000);
		///currentJoint->IKOffset.x = (float)ui->xSlider->value() / 100;
		ui->xLabel->setText(QString::fromStdString(std::to_string(currentJoint->position.x)));
	}
}*/
}

void GUI::on_ySlider_sliderMoved(int position)
{
	if (currentJoint) {
			FABRIK(currentJoint, 0, position / 1000, 0);
			///currentJoint->IKOffset.x = (float)ui->xSlider->value() / 100;
			ui->yLabel->setText(QString::fromStdString(std::to_string(currentJoint->position.y)));
		
	}
}

void GUI::on_zSlider_sliderMoved(int position)
{
	if (currentJoint) {
			FABRIK(currentJoint, 0, 0, position / 1000);
			///currentJoint->IKOffset.x = (float)ui->xSlider->value() / 100;
			ui->zLabel->setText(QString::fromStdString(std::to_string(currentJoint->position.z)));
		
	}
}

void GUI::on_playButton_clicked()
{
	this->renderWindow->play = !(this->renderWindow->play);
}
/*
tVector3  getAngles(tVector3 cords) {
	tVector3 result;
	result.x = acos(cords.x);
	result.y = acos(cords.y);
	result.z = acos(cords.z);
	return result;
}
tVector3 crossProduct(tVector3 vect_A, tVector3 vect_B)

{
	tVector3 cross;
	cross.x = vect_A.y * vect_B.z - vect_A.z * vect_B.y;
	cross.y = vect_A.z * vect_B.x - vect_A.x * vect_B.z;
	cross.z = vect_A.x * vect_B.y - vect_A.y * vect_B.x;
	return cross;
}*/
void  GUI::calculateRotation(root *joint) {
	
	if (!joint->isroot) {
	
	}
		for (int i = 0;i < joint->children.size();i++) {
			
			calculateRotation(joint->children[i]);
		}
	
}/*
void  GUI::calculatePositon(root *joint) {


	if (joint->isroot) {
		joint->position.x = joint->offset.x;

		joint->position.y = joint->offset.y;

		joint->position.z = joint->offset.z;

	}
	else {
		joint->position.x =joint->parent->position.x+ joint->offset.x;

		joint->position.y = joint->parent->position.y + joint->offset.y;

		joint->position.z = joint->parent->position.z + joint->offset.z;
	
	}


	for (int i = 0;i < joint->children.size();i++) {
		calculatePositon(joint->children[i]);
	}

}

std::vector<tVector3> GetJacobianTranspose(root *joint,tVector3 endEffectorPos) {
	std::vector<tVector3> transposed;
	tVector3 DOF;
	DOF = crossProduct({ 1,1,1 }, endEffectorPos.operator-(joint->position));
	if (!joint->isroot) {
		GetJacobianTranspose(joint->parent, endEffectorPos);
	}
	transposed.push_back(DOF);
	return transposed;
}
std::vector<float> GetDeltaOrientation() {

	std::vector<tVector3> Jt = GetJacobianTranspose();
	V = targetPosition — endEffectorPosition;
	std::vector<float> dO = Jt * V; // Matrix-Vector Mult.
	return dO;
}


void JacobianIK() {
	while (abs(endEffectorPosition — targetPosition) > EPS) {
		dO = GetDeltaOrientation();
		O += dO * h; // T=O+dO*h
	}
}*/
void GUI::on_stopButton_clicked()
{
	this->renderWindow->play = !(this->renderWindow->play);
	this->renderWindow->currentFrame=0;
}

void GUI::on_nextButton_clicked()
{
	this->renderWindow->currentFrame++;
	if (this->renderWindow->currentFrame >= frames.size()) {
		this->renderWindow->currentFrame = 0;
	}

}

void GUI::on_prevButton_clicked()
{
	this->renderWindow->currentFrame--;
	if (this->renderWindow->currentFrame == -1) {
		this->renderWindow->currentFrame = frames.size()-1;
	}

}


void changeChildren(root * joint, tVector3 endPos) {

	joint->position = joint->position + endPos;
	for (int i = 0;i < joint->children.size();i++) {
		changeChildren(joint->children[i], endPos);
	}


}


void GUI::FABRIK(root *joint, float changeX, float changeY, float changeZ) {

	root* temp = currentJoint;
	tVector3 rootPos = skeleton->position;

	tVector3 endPos = currentJoint->position;
	std::vector<root*> all;
	all.push_back(temp);
	std::vector<float> allDist;
	while (!temp->isroot) {
		float dist = sqrt(pow(temp->position.x - temp->parent->position.x, 2) + pow(temp->position.y - temp->parent->position.y, 2) + pow(temp->position.z - temp->parent->position.z, 2));
		temp = temp->parent;
		allDist.push_back(dist);
       //  std::cout <<dist<<std::endl;
	}

	currentJoint->position.x = currentJoint->position.x + changeX;
	currentJoint->position.y = currentJoint->position.y + changeY;
	currentJoint->position.z = currentJoint->position.z + changeZ;
	temp = currentJoint;
	int j = 0;
	while (!temp->isroot) {

       
		tVector3 dir = temp->parent->position.operator-(temp->position);
		if(dir.x!=0&& dir.y != 0 && dir.z != 0)
		dir.norm();
		temp->parent->position = temp->position +( dir*allDist[j]);
		temp = temp->parent;
		all.push_back(temp);
		j++;
	}
	skeleton->position = rootPos;
	for (int i = all.size()-1;i > 0;i--) {
	
		tVector3 dir = all[i-1]->position - all[i]->position;

		if (dir.x != 0 && dir.y != 0 && dir.z != 0)
		dir.norm();
		all[i - 1]->position = all[i]->position + dir*allDist[i-1];
	}
	endPos = currentJoint->position - endPos;
	for(int i=0;i<currentJoint->children.size();i++)
	changeChildren(currentJoint->children[i], endPos);


 
}
 


void GUI::on_xSlider_valueChanged(int value)
{
	if (currentJoint) {
		
            FABRIK(currentJoint, value /1000,0,0);
			///currentJoint->IKOffset.x = (float)ui->xSlider->value() / 100;
			ui->xLabel->setText(QString::fromStdString(std::to_string(currentJoint->position.x)));
		
	}
}

void GUI::on_translationMode_toggled(bool checked)
{
    this->renderWindow->animation=checked;

}

void GUI::on_rotationMode_toggled(bool checked)
{
}
