#include "GLViewFinalProject.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "SDL_image.h"


using namespace Aftr;

GLViewFinalProject* GLViewFinalProject::New(const std::vector< std::string >& args)
{
	GLViewFinalProject* glv = new GLViewFinalProject(args);
	glv->init(Aftr::GRAVITY, Vector(0, 0, -1.0f), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE);
	glv->onCreate();
	return glv;
}


GLViewFinalProject::GLViewFinalProject(const std::vector< std::string >& args) : GLView(args)
{
	//Initialize any member variables that need to be used inside of LoadMap() here.
	//Note: At this point, the Managers are not yet initialized. The Engine initialization
	//occurs immediately after this method returns (see GLViewFinalProject::New() for
	//reference). Then the engine invoke's GLView::loadMap() for this module.
	//After loadMap() returns, GLView::onCreate is finally invoked.

	//The order of execution of a module startup:
	//GLView::New() is invoked:
	//    calls GLView::init()
	//       calls GLView::loadMap() (as well as initializing the engine's Managers)
	//    calls GLView::onCreate()

	//GLViewFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewFinalProject::onCreate()
{
	//GLViewFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
	//At this point, all the managers are initialized. That is, the engine is fully initialized.

	if (this->pe != NULL)
	{
		//optionally, change gravity direction and magnitude here
		//The user could load these values from the module's aftr.conf
		this->pe->setGravityNormalizedVector(Vector(0, 0, -1.0f));
		this->pe->setGravityScalar(Aftr::GRAVITY);
	}
	this->setActorChaseType(STANDARDEZNAV); //Default is STANDARDEZNAV mode
	//this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}


GLViewFinalProject::~GLViewFinalProject()
{
	//Implicitly calls GLView::~GLView()
}


void GLViewFinalProject::updateWorld()
{
	GLView::updateWorld(); //Just call the parent's update world first.
	//If you want to add additional functionality, do it after
	//this call.
}


void GLViewFinalProject::onResizeWindow(GLsizei width, GLsizei height)
{
	GLView::onResizeWindow(width, height); //call parent's resize method.
}


void GLViewFinalProject::onMouseDown(const SDL_MouseButtonEvent& e)
{
	if (closeText) {
		GLView::onMouseDown(e);
	}
}


void GLViewFinalProject::onMouseUp(const SDL_MouseButtonEvent& e)
{
	GLView::onMouseUp(e);
}


void GLViewFinalProject::onMouseMove(const SDL_MouseMotionEvent& e)
{
	GLView::onMouseMove(e);
}


void GLViewFinalProject::onKeyDown(const SDL_KeyboardEvent& key)
{
	GLView::onKeyDown(key);
	if (key.keysym.sym == SDLK_0)
		this->setNumPhysicsStepsPerRender(1);

	if (key.keysym.sym == SDLK_w && closeText)
	{
		this->cam->moveInLookDirection();
		this->cam->setPosition(this->cam->getPosition().x, this->cam->getPosition().y, 10);
		collision(this->cam);

	}

	if (key.keysym.sym == SDLK_s && closeText)
	{
		this->cam->moveOppositeLookDirection();
		this->cam->setPosition(this->cam->getPosition().x, this->cam->getPosition().y, 10);
		collision(this->cam);

	}

	if (key.keysym.sym == SDLK_a && closeText)
	{
		this->cam->moveLeft();
		collision(this->cam);


	}

	if (key.keysym.sym == SDLK_d && closeText)
	{
		this->cam->moveRight();
		collision(this->cam);

	}

	if (key.keysym.sym == SDLK_UP) {
		this->cam->moveRelative(Vector(0, 0, 1));
	}

	if (key.keysym.sym == SDLK_DOWN) {
		this->cam->moveRelative(Vector(0, 0, -1));
	}

	if (key.keysym.sym == SDLK_RETURN) {
		for (int i = 0; i < worldLst->size(); i++) {
			if (this->worldLst->at(i)->getLabel() == "text0") {
				this->worldLst->at(i)->isVisible = false;
				closeText = 1;
			}
		}
	}

	if (key.keysym.sym == SDLK_0 && closeText) {
		for (int i = 0; i < worldLst->size(); i++) {
			if (this->worldLst->at(i)->getLabel() == "key") {
			this->worldLst->at(i)->isVisible = true;
			}
		}
		
	}
}


void GLViewFinalProject::onKeyUp(const SDL_KeyboardEvent& key)
{
	GLView::onKeyUp(key);
}


void Aftr::GLViewFinalProject::loadMap()
{
	this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
	this->actorLst = new WorldList();
	this->netLst = new WorldList();

	ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
	ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
	ManagerOpenGLState::enableFrustumCulling = false;
	Axes::isVisible = false;
	this->glRenderer->isUsingShadowMapping(true); //set to TRUE to enable shadow mapping, must be using GL 3.2+

	this->cam->setPosition(0, -11, 10);
	this->cam->rotateAboutGlobalZ(90.0f * DEGtoRAD);
	lastPos = cam->getPosition();

	std::string shinyRedPlasticCube(ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl");
	std::string wheeledCar(ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl");
	std::string grass(ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl");
	std::string human(ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl");

	//SkyBox Textures readily available
	std::vector< std::string > skyBoxImageNames; //vector to store texture paths
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg");
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
	//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

	//{
	//   //Create a light
	//   float ga = 0.1f; //Global Ambient Light level for this module
	//   ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
	//   WOLight* light = WOLight::New();
	//   light->isDirectionalLight( true );
	//   light->setPosition( Vector( 0, 0, 100 ) );
	//   //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
	//   //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
	//   light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
	//   light->setLabel( "Light" );
	//   worldLst->push_back( light );
	//}

	{
		//Create the SkyBox
		WO* wo = WOSkyBox::New(skyBoxImageNames.at(0), this->getCameraPtrPtr());
		wo->setPosition(Vector(0, 0, 0));
		wo->setLabel("Sky Box");
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	{
		////Create the infinite grass plane (the floor)
		WO* wo = WO::New(grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
		grassSkin.getMultiTextureSet().at(0).setTexRepeats(5.0f);
		grassSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
		grassSkin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
		grassSkin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
		grassSkin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
			});
		wo->setLabel("Grass");
		worldLst->push_back(wo);
	}

		// floor
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(7.5, 7.5, 0.5), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Morion.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//wall 1
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(0.5, 10, 10), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(15, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/wallpaper.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//wall 2
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(0.5, 10, 10), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(-15, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/wallpaper.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//wall 3
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(10, 0.5, 10), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 15, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/wallpaper.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//wall 4
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(10, 0.5, 10), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, -15, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/wallpaper.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//ceiling
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(7.5, 7.5, 0.5), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 0, 20));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Morion.jpg").value());
		floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
		wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//door
	{
		WO* wo = WO::New("../mm/models/door/door.obj", Vector(.007, .007, .007), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(0, 14.45, 8));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		
		wo->upon_async_model_loaded([wo]()
			{
				
		//		ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Verona_PO_Morion_albedo.jpg").value());
		//floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstSMOOTH);
		//wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

		//desk
	{
		WO* wo = WO::New("../mm/models/desk/scene.gltf", Vector(1.2, 1.2, 1.2), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(12, 5, 3.9));
		wo->rotateAboutGlobalZ(-90.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;

		wo->upon_async_model_loaded([wo]()
			{

				//		ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Verona_PO_Morion_albedo.jpg").value());
				//floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstSMOOTH);
				//wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
			}
		);
		worldLst->push_back(wo);
	}

	//drawers
	{
		WO* wo = WO::New("../mm/models/drawers/scene.gltf", Vector(4, 4, 4), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(-10.5, 6, 5));
		//wo->rotateAboutGlobalZ(-180.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//desk books
	{
		WO* wo = WO::New("../mm/models/books/books.obj", Vector(0.003, 0.003, 0.003), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(11.25, 5, 7));
		wo->rotateAboutGlobalZ(90.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//desk chair
	{
		WO* wo = WO::New("../mm/models/chair2/scene.gltf", Vector(0.0215, 0.0215, 0.0215), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(9.5, 5.245, 5.05));
		wo->rotateAboutGlobalZ(77.5f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//desk lamp
	{
		WO* wo = WO::New("../mm/models/lamp/scene.gltf", Vector(0.04, 0.04, 0.04), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(12.5, 2.3, 8));
		wo->rotateAboutGlobalZ(125.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//phone
	{
		WO* wo = WO::New("../mm/models/phone/scene.gltf", Vector(0.08, 0.08, 0.08), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(12, 8, 7.35));
		wo->rotateAboutGlobalZ(-35.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//starting text
	{
		WO* wo = WO::New("../mm/models/startup-text/startup-text.obj", Vector(0.02, 0.02, 0.02), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(0, 0, 10));
		wo->setLabel("text0");
		//wo->rotateAboutGlobalZ(-35.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//key
	{
		WO* wo = WO::New("../mm/models/key/scene.gltf", Vector(0.001, 0.001, 0.001), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(-8.5, 7.5 , 4.2));
		wo->rotateAboutGlobalX(90.0f * DEGtoRAD);
		wo->rotateAboutGlobalZ(23.5f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->isVisible = false;
		wo->setLabel("key");
		worldLst->push_back(wo);
	}

	//switch
	{
		WO* wo = WO::New("../mm/models/switch/scene.gltf", Vector(1, 1, 1), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(3, -13.5, 4.2));
		//wo->rotateAboutGlobalX(90.0f * DEGtoRAD);
		wo->rotateAboutGlobalZ(180.0f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->setLabel("switch");
		worldLst->push_back(wo);
	}

	//armchair
	{
		WO* wo = WO::New("../mm/models/chair3/scene.gltf", Vector(0.035, 0.035, 0.035), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(-7, -9, 6));
		//wo->rotateAboutGlobalX(90.0f * DEGtoRAD);
		wo->rotateAboutGlobalZ(53.5f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}

	//armchair2
	{
		WO* wo = WO::New("../mm/models/chair3/scene.gltf", Vector(0.035, 0.035, 0.035), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(7, -9, 6));
		//wo->rotateAboutGlobalX(90.0f * DEGtoRAD);
		wo->rotateAboutGlobalZ(140.5f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}


	//small table
	{
		WO* wo = WO::New("../mm/models/small-table/scene.gltf", Vector(0.6, 0.6, 0.6), MESH_SHADING_TYPE::mstSMOOTH);
		wo->setPosition(Vector(0, -10, 4));
		//wo->rotateAboutGlobalX(90.0f * DEGtoRAD);
		wo->rotateAboutGlobalZ(140.5f * DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		worldLst->push_back(wo);
	}


	//{
	//	//chair
	//	WO* wo = WO::New("../mm/models/chair/scene.gltf", Vector(2, 2, 2), MESH_SHADING_TYPE::mstSMOOTH);
	//	wo->setPosition(Vector(10, -10, 4));
	//	wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;

	//	wo->upon_async_model_loaded([wo]()
	//		{

	//			//		ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Verona_PO_Morion_albedo.jpg").value());
	//			//floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstSMOOTH);
	//			//wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
	//		}
	//	);
	//	worldLst->push_back(wo);
	//}

	//{
	//	//barrel
	//	WO* wo = WO::New("../mm/models/barrel/scene.gltf", Vector(0.05, 0.05, 0.05), MESH_SHADING_TYPE::mstSMOOTH);
	//	wo->setPosition(Vector(-10, 10, 4));
	//	wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;

	//	wo->upon_async_model_loaded([wo]()
	//		{

	//			//		ModelMeshSkin floorSkin(ManagerTex::loadTexAsync("../mm/images/Verona_PO_Morion_albedo.jpg").value());
	//			//floorSkin.setMeshShadingType(MESH_SHADING_TYPE::mstSMOOTH);
	//			//wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(floorSkin);
	//		}
	//	);
	//	worldLst->push_back(wo);
	//}

	//{
	//    //Create a light
	//    WOLight* light = WOLight::New();
	//    light->isPointLight(true);
	//    light->setPosition(Vector(0, 0, 19));
	//    //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
	//    //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
	//    light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({ 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD));
	//    light->setLabel("Light1");
	//    worldLst->push_back(light);
	//}

	//{
	//   //Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
	//   WO* wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
	//   ((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
	//   wo->setPosition( Vector(0,0,0) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   //Create the infinite grass plane that uses NVIDIAPhysX(the floor)
	//   WO* wo = WONVStaticPlane::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0, 0 ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 ).getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   //Create the infinite grass plane (the floor)
	//   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0, 50.0f ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0.5f, 75.0f ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   WO* wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue( "sharedmultimediapath" ) + "/models/sphereRp5.wrl", Vector( 1.0f, 1.0f, 1.0f ), mstSMOOTH );
	//   wo->setPosition( 0, 0, 100.0f );
	//   wo->setLabel( "Sphere" );
	//   this->worldLst->push_back( wo );
	//}

	//{
	//   WO* wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
	//   ((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
	//   ((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
	//   wo->setPosition( Vector( 20, 20, 20 ) );
	//   wo->setLabel( "Paladin" );
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo );
	//   netLst->push_back( wo );
	//   this->setActor( wo );
	//}
	//
	//{
	//   WO* wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
	//   wo->setPosition( Vector( 20, 10, 20 ) );
	//   wo->isVisible = false; //hide the WOHuman's bounding box
	//   ((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
	//   wo->setLabel( "Human Cyborg" );
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo ); //Push the WOHuman as an actor
	//   netLst->push_back( wo );
	//   this->setActor( wo ); //Start module where human is the actor
	//}

	//{
	//   //Create and insert the WOWheeledVehicle
	//   std::vector< std::string > wheels;
	//   std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   WO* wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
	//   wo->setPosition( Vector( 5, -15, 20 ) );
	//   wo->setLabel( "Car 1970s Beater" );
	//   ((WOODE*)wo)->mass = 200;
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo );
	//   this->setActor( wo );
	//   netLst->push_back( wo );
	//}

	//Make a Dear Im Gui instance via the WOImGui in the engine... This calls
	//the default Dear ImGui demo that shows all the features... To create your own,
	//inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).

}


void GLViewFinalProject::createFinalProjectWayPoints()
{
	// Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
	WayPointParametersBase params(this);
	params.frequency = 5000;
	params.useCamera = true;
	params.visible = true;
	WOWayPointSpherical* wayPt = WOWayPointSpherical::New(params, 3);
	wayPt->setPosition(Vector(50, 0, 3));
	worldLst->push_back(wayPt);
}

void GLViewFinalProject::collision(Aftr::Camera* cam)
{

	if (cam->getPosition().x >= 12) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().x <= -12) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().y >= 12) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().y <= -12) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().x > 9 && cam->getPosition().y > -6 && cam->getPosition().y < 6) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().x > 6 && cam->getPosition().y > -2 && cam->getPosition().y < 3) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().x < -8 && cam->getPosition().y > -1) {
		cam->setPosition(lastPos);
	}

	if (cam->getPosition().x < -7 && cam->getPosition().y > 5 && cam->getPosition().y < 9) {
		cam->setPosition(lastPos);
	}

	lastPos = cam->getPosition();
}

