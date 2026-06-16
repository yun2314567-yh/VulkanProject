//#include"helloTriangle.h"

#include"myEngine/myEngine-App.h"



int main()
{
		


	myEngine::App app;
	app.setCameraInfo(glm::vec3(0,3,5));
	app.setPerspectionMatInfo( 0.1, 20);
	app.setModelInfo("E:\\waveFeiXue/Model/feixue.fbx");
	app.run();
		return 0;
}
