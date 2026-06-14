//#include"helloTriangle.h"

#include"myEngine/myEngine-App.h"



int main()
{
	//myEngine::Engine m;
	
	
		/*HelloTriangle app;
		std::vector<Vertex> vertices = 
		{   {{-0.5f , -0.5f,0 } , {1.0f , 0.0f , 0.0f },{1,0} },
			{{0.5f , -0.5f,0 } , {1.0f , 1.0f , 0.0f },{0,0}} ,
            {{0.5f , 0.5f ,0} , {0.0f , 1.0f , 0.0f },{0,1}} ,
            {{-0.5f , 0.5f,0 } , {0.0f , 0.0f , 1.0f },{1,1}},
			
			{{-0.5f , -0.5f,-.3 } , {0.0f , 1.0f , 0.0f },{1,0} },
			{{0.5f , -0.5f,-.3 } , {1.0f , 1.0f , 0.0f },{0,0}} ,
			{{0.5f , 0.5f ,-.3} , {0.0f ,  0.0f, 1.0f },{0,1}} ,
			{{-0.5f , 0.5f,-.3 } , {0.0f , 0.0f , 1.0f },{1,1}},
		  };
		std::vector<uint16_t> indices =
		{
			0,1,2,
			2,3,0,

			4,5,6,
			6,7,4
		};
		
		app.setTexPath("textures/bricks2.jpg");
		app.setMVPInfo(glm::vec3(2., 2., 2.), 45.f, 0.1, 10);
		app.vertices = std::move(vertices);
		app.indices = std::move(indices);
		app.run();*/


	myEngine::App app;
	app.setCameraInfo(glm::vec3(0,3,5));
	app.setPerspectionMatInfo( 0.1, 20);
	app.setModelInfo("E:\\waveFeiXue/Model/feixue.fbx");
	app.run();
		return 0;
}
