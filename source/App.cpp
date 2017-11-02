/** \file App.cpp */
#include "App.h"
#include "Raycaster.h"


// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.caption             = argv[0];

    // Set enable to catch more OpenGL errors
    // settings.window.debugContext     = true;

    // Some common resolutions:
    // settings.window.width            =  854; settings.window.height       = 480;
    // settings.window.width            = 1024; settings.window.height       = 768;
    settings.window.width               = 1280; settings.window.height       = 720;
    //settings.window.width             = 1920; settings.window.height       = 1080;
    // settings.window.width            = OSWindow::primaryDisplayWindowSize().x; settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;

    // Set to true for a significant performance boost if your app can't render at 60fps, or if
    // you *want* to render faster than the display.
    settings.window.asynchronous        = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();
    settings.screenCapture.outputDirectory = "../journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;

    return App(settings).run();
}

void makeHeightField(shared_ptr<G3D::Image> img, float horizScale, float yScale)
{
	TextOutput * to = new TextOutput("model/heightmap.off");
	
	//each pixel will be a vertice, the image will be surrounded by an outer ring of vertices to form the border
	int numFaces = (img->width() + 1)*(img->height()+1);
	int numVertices = (img->width() + 2)*(img->height()+2);
	int numEdges = (img->width()+1) * img->height() * 2;

	to->printf("OFF\n#\n# heightfield.off\n# \n");
  to->printf("%i %i %i\n",numVertices,numFaces,numEdges);
	
	//print the vertices
	
	//print top border 
	for(int i = 0; i < img->width()+2; i++)
	{
		float x = (i)*horizScale - img->width()/2;
		float y = -yScale; 
		float z = img->height()*horizScale;
		to->printf("%f %f %f\n",x,y,z);

	}
	
	for(int j = 0; j < img->height(); j++)
	{
		//print vertice in left border
		float x = -img->width()/2;
		float y = -yScale; 
		float z = (img->height()-(j+1))*horizScale;
		to->printf("%f %f %f\n",x,y,z);

		for(int i = 0; i < img->width(); i++)
		{
			Color1unorm8 color; 
			img->get(G3D::Point2int32(i,j),color);
			float x = (i+1)*horizScale - img->width()/2;
			float y = float(color.value) * yScale -yScale; 
			float z = (img->height()-(j+1))*horizScale;
			to->printf("%f %f %f\n",x,y,z);
		}
		
		//print vertice in right border
		 x = (img->width()+1)*horizScale - img->width()/2;
		 y = -yScale; 
		 z = (img->height()-(j+1))*horizScale;
		to->printf("%f %f %f\n",x,y,z);

	}
	//print bottom border 
	for(int i = 0; i < img->width()+2; i++)
	{
		float x = (i)*horizScale - img->width()/2;
		float y = -yScale; 
		float z = 0.0;
		to->printf("%f %f %f\n",x,y,z);

	}
	//time for the faces
	for(int j = 0; j < img->height()+1; j++)
	{
		for(int i = 0; i < img->width()+1; i++)
		{
			int topLeftV = j*(img->width()+2) + i;
			int topRightV = topLeftV + 1; 
			int bottomLeftV = topLeftV + img->width() +2;
			int bottomRightV = bottomLeftV + 1; 
			
			to->printf("4 %i %i %i %i\n",topLeftV,topRightV,bottomRightV,bottomLeftV);

		}
	}
	 
	to->commit();
	delete to;

}

void makeGlass(int slices)
{
	std::vector<G3D::Point2int32> curve;
	curve.push_back(G3D::Point2int32(269,259));
	curve.push_back(G3D::Point2int32(286,255));
	curve.push_back(G3D::Point2int32(300,254));
	curve.push_back(G3D::Point2int32(317,249));
	curve.push_back(G3D::Point2int32(332,246));
	curve.push_back(G3D::Point2int32(344,220));
	curve.push_back(G3D::Point2int32(353,202));
	curve.push_back(G3D::Point2int32(361,175));
	curve.push_back(G3D::Point2int32(363,154));
	curve.push_back(G3D::Point2int32(360,101));
	curve.push_back(G3D::Point2int32(354,74));
	curve.push_back(G3D::Point2int32(346,49));
	curve.push_back(G3D::Point2int32(340,32));
	curve.push_back(G3D::Point2int32(347,31));
	curve.push_back(G3D::Point2int32(355,46));
	curve.push_back(G3D::Point2int32(364,69));
	curve.push_back(G3D::Point2int32(368,96));
	curve.push_back(G3D::Point2int32(372,131));
	curve.push_back(G3D::Point2int32(374,154));
	curve.push_back(G3D::Point2int32(369,181));
	curve.push_back(G3D::Point2int32(360,206));
	curve.push_back(G3D::Point2int32(348,227));
	curve.push_back(G3D::Point2int32(321,258));
	curve.push_back(G3D::Point2int32(304,267));
	curve.push_back(G3D::Point2int32(281,275));
	curve.push_back(G3D::Point2int32(272,279));
	curve.push_back(G3D::Point2int32(269,290));
	curve.push_back(G3D::Point2int32(264,322));
	curve.push_back(G3D::Point2int32(262,372));
	curve.push_back(G3D::Point2int32(266,415));
	curve.push_back(G3D::Point2int32(272,444));
	curve.push_back(G3D::Point2int32(280,456));
	curve.push_back(G3D::Point2int32(498,467));
	curve.push_back(G3D::Point2int32(323,478));
	curve.push_back(G3D::Point2int32(344,461));
	curve.push_back(G3D::Point2int32(346,490));
	curve.push_back(G3D::Point2int32(274,490));	
	TextOutput * to = new TextOutput("model/glass.off");
	
	//basically this is all analogous to the cylinder, only innerCurve.size() + outerCurver.size() is one greater than segments
	int numVertices = slices*curve.size() + 2;
	int numFaces = slices*(curve.size() +1);
	int numEdges = slices*(curve.size()*2+1); 

	to->printf("OFF\n#\n# glass.off\n# \n");
	to->printf("%i %i %i\n",numVertices,numFaces,numEdges);

	float PI = 3.14195;
	
	//print the vertices
	
for(std::vector<G3D::Point2int32>::iterator it = curve.begin(); it != curve.end(); ++it) 
{	
		for(int i = 0; i < slices; i++)
		{


			float angle = (2*PI/slices)*i;
			float x = cos(angle)*((*it).x-260);
			float y = 200-(*it).y; 
			float z = sin(angle)*((*it).x-260); 
			to->printf("%f %f %f\n",x,y,z);
		}
	}
	
	//hub of bottom cap 
	to->printf("0 %f 0\n",200.0-250.0);
	
	//hub of top cap
	to->printf("0 %f 0\n",200.0-490.0);

	
	//print the faces
	for(int j = 0; j < int(curve.size()-1); j++)
	{
		for(int i = 0; i < slices; i++)
		{
			int topLeftV = ((j+1)*slices)+i;
			int topRightV = i+1 < slices ? ((j+1)*slices)+i+1 : (j+1)*slices; 
			int bottomLeftV = (j*slices)+i;
			int bottomRightV = i+1 < slices ? (j*slices)+i+1 : j*slices; 
			
			to->printf("4 %i %i %i %i\n",topLeftV,topRightV,bottomRightV,bottomLeftV);
		}
	}	
	
	//now do the caps
	int bottomHub = slices*curve.size();
	int topHub = slices*curve.size()+1;
	
	for(int i = 0; i < slices; i++)
	{
		int leftV = i;
		int rightV = i+1 < slices ? i+1 : 0; 		
		to->printf("3 %i %i %i\n",leftV,rightV,bottomHub);
		
		int otherLeftV = (curve.size()-1)*slices +i;
		int otherRightV = i+1 < slices ? (curve.size()-1)*slices +i+1 : (curve.size()-1)*slices;
		
		to->printf("3 %i %i %i\n",otherLeftV,topHub,otherRightV);

	}

	to->commit();
	delete to;
	
		
}
//slices is number of slices as in a cake, how close to a true cylinder it will be 
//segments is vertical divisions
void makeCylinder(float radius, float height, int slices, int segments)
{
	TextOutput * to = new TextOutput("model/cylinder.off");
	
	//one to originate each edge in the ring, plus an extra vertice per slice to cap, plus two for the center of top and bottom
	int numVertices = slices*(segments+1)+2;
	int numFaces = slices*(segments+2);
	//one for each edge in the ring, plus one vertical edge per slice,and an extra ring on the caps. then two extra edges on the caps for spokes
	int numEdges = slices*(segments*2+3);
	
	to->printf("OFF\n#\n# cylinder.off\n# \n");
  to->printf("%i %i %i\n",numVertices,numFaces,numEdges);
	
	float PI = 3.14195;
	
	//print the vertices
	
	for(int j = 0; j <segments+1; j++)
	{
		for(int i = 0; i < slices; i++)
		{

			float angle = (2*PI/slices)*i;
			float x = cos(angle)*radius;
			float y = (height/segments)*j-height/2.0; 
			float z = sin(angle)*radius;
			to->printf("%f %f %f\n",x,y,z);
		}
	}
	
	//hub of bottom cap 
	to->printf("0 0 0\n");
	
	//hub of top cap
	to->printf("0 %f 0\n", height);

	
	//print the faces
	for(int j = 0; j <segments; j++)
	{
		for(int i = 0; i < slices; i++)
		{
			int topLeftV = ((j+1)*slices)+i;
			int topRightV = i+1 < slices ? ((j+1)*slices)+i+1 : (j+1)*slices; 
			int bottomLeftV = (j*slices)+i;
			int bottomRightV = i+1 < slices ? (j*slices)+i+1 : j*slices; 
			
			to->printf("4 %i %i %i %i\n",topLeftV,topRightV,bottomRightV,bottomLeftV);
		}
	}	
	
	//now do the caps
	int bottomHub = slices*(segments+1);
	int topHub = slices*(segments+1)+1;
	
	for(int i = 0; i < slices; i++)
	{
		int leftV = i;
		int rightV = i+1 < slices ? i+1 : 0; 		
		to->printf("3 %i %i %i\n",leftV,rightV,bottomHub);
		
		int otherLeftV = segments*slices +i;
		int otherRightV = i+1 < slices ? segments*slices +i+1 : segments*slices;
		
		to->printf("3 %i %i %i\n",otherLeftV,topHub,otherRightV);

	}

	to->commit();
	delete to;

}

App::App(const GApp::Settings& settings) : GApp(settings) {
}


// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
	
	
	TextOutput * to = new TextOutput("scene/myScene3.Scene.Any");
	
	to->printf("// -*- c++ -*-\n");
	to->printf("	{\n");
	to->printf("    name = \"My Scene 3 (Staircase)\";");
	    
		to->printf(" 	    models = {\n");
		to->printf("        stair = ArticulatedModel::Specification {\n");
	  to->printf("           filename = \"model/crate/crate.obj\";");
		to->printf("             preprocess = {\n");
		to->printf(" 							setMaterial(all(), \"wood.jpg\");\n");
		to->printf(" 							transformGeometry(all(), Matrix4::scale(1.0, 0.1, 0.1));\n");
		to->printf("             };\n");
		to->printf(" 	       };\n");
		to->printf("     };\n");
	    
		to->printf("     entities = {\n");
		to->printf("        skybox = Skybox {\n");
		to->printf(" 					 texture = \"cubemap/whiteroom/whiteroom-*.png\";\n");
		to->printf("        };\n");
	 	to->printf("       sun = Light {\n");
	  	to->printf("           attenuation = ( 0, 0, 1 );\n");
		to->printf("             bulbPower = Power3(4e+006);\n");
		to->printf("             frame = CFrame::fromXYZYPRDegrees(-15, 207, -41, -164, -77, 77);\n");
		to->printf("             shadowMapSize = Vector2int16(2048, 2048);\n");
		to->printf("             spotHalfAngleDegrees = 5;\n");
		to->printf("             rectangular = true;\n");
		to->printf("             type = \"SPOT\";\n");
	 	to->printf("        };\n");
		for(int i = 0; i < 50; i++)
		{
			to->printf("         cube%u = VisibleEntity {\n",i);
			to->printf("             model = \"stair\";\n");
			float height = i*.05;
			float rot = i*7.5;
			to->printf("             frame = CFrame::fromXYZYPRDegrees(0, %f, 0, %f, 0, 0);\n",height,rot);
			to->printf("         };\n");
		}
		to->printf("         camera = Camera {\n");
		to->printf("             frame = CFrame::fromXYZYPRDegrees(0, 0, 5);\n");
		to->printf("         };\n");
		to->printf("     };\n");
		to->printf(" };\n");
	
		to->commit();
		
		delete to;
		
		
		debugPrintf("Target frame rate = %f Hz\n", realTimeTargetDuration());
	//	debugPrint("System::appDataDir() %s",System::appDataDir());
    GApp::onInit();
    setFrameDuration(1.0f / 60.0f);

    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default scene here.
    
    showRenderingStats      = false;

    makeGUI();
    // For higher-quality screenshots:
    // developerWindow->videoRecordDialog->setScreenShotFormat("PNG");
    // developerWindow->videoRecordDialog->setCaptureGui(false);

    loadScene(
#       ifndef G3D_DEBUG
            "G3D Sponza"
#       else
            "G3D Simple Cornell Box" // Load something simple
#       endif
        //"G3D Simple Cornell Box (Spheres)" // Load something simple
        //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
        );
}
float heightVal = 5.0;
float radiusVal = 1.0;

int renderWidth = 1; 
int renderHeight = 1;

bool fixedPrimitives = false;
bool multithreading = false;
int indirectRays = 0;

void App::makeGUI() {
    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);

    GuiPane* infoPane = debugPane->addPane("Cylinder time", GuiTheme::ORNATE_PANE_STYLE);
		// add cylinder to scene
		infoPane->addLabel("Create a cylinder");

		infoPane->addNumberBox("height", &heightVal, " units")->setUnitsSize(30);
		infoPane->addNumberBox("radius", &radiusVal, " units")->setUnitsSize(30);
		infoPane->addButton("Go", [this, hv = &heightVal, rv = &radiusVal]() { 
			debugPrintf("User entered some radius %f and height %f\n",*rv,*hv);
			makeCylinder(*rv,*hv,24,10);
			ArticulatedModel::clearCache();
			loadScene("My Scene 5 (My new cylinder mesh)");
			
		 });    
    infoPane->pack();
		
		GuiPane* renderingPane = debugPane->addPane("Test Max Foley's rendering algorithm");
		renderingPane->addLabel("Parameters");
		renderingPane->addNumberBox("width",&renderWidth," pixels");
		renderingPane->addNumberBox("height",&renderHeight," pixels");
		renderingPane->addCheckBox("use fixed primitives",&fixedPrimitives);
		renderingPane->addCheckBox("use multithreading",&multithreading);
		renderingPane->addNumberBox("how many indirect rays do you want to use",&indirectRays," rays");
		
		
		renderingPane->addButton("Go",
		 	[this, rw = &renderWidth, rh = &renderHeight, fp = &fixedPrimitives, mt = &multithreading, ir = &indirectRays]() { 
				App::MaxFoleyRender(*rw,*rh,*fp,*mt,*ir);
			
		 });    
    
		debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}

void App::MaxFoleyRender(int width, int height, bool fixedPrimitives, bool multithreading, int indirectRays)
{
	//display a waiting screen

	//make a new image with the right resolution
	shared_ptr<Image> img = Image::create(width,height,G3D::ImageFormat::RGB8());
		
	//run the helper function
	Raycaster raycaster = Raycaster();

	raycaster.RenderImage(activeCamera(), scene(), img, fixedPrimitives, multithreading, indirectRays);
	
	//gamma encoding 
	
	//save the image 
	img->save("maxrender.jpg");
	
	//render the image
	//shared_ptr<Texture> texture = make_shared(Texture());
	//texture = texture->toImage(img);
	
	
}


// This default implementation is a direct copy of GApp::onGraphics3D to make it easy
// for you to modify. If you aren't changing the hardware rendering strategy, you can
// delete this override entirely.
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
	
    if (! scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (!rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }


    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);
    m_gbuffer->resize(m_framebuffer->width(), m_framebuffer->height());
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_renderer->render(rd, m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : shared_ptr<Framebuffer>(),
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces);

    // Debugging visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());

        // Call to make the App show the output of debugDraw(...)
        drawDebugShapes();
        const shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : shared_ptr<Entity>();
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        // Post-processed special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

        m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_POSITION_CHANGE),
            m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
            m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    rd->clear();

    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.colorGuardBandThickness.x + settings().hdrFramebuffer.depthGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x, 
        Texture::opaqueBlackIfNull(notNull(m_gbuffer) ? m_gbuffer->texture(GBuffer::Field::SS_POSITION_CHANGE) : nullptr),
        activeCamera()->jitterMotion());
}


void App::onAI() {
    GApp::onAI();
    // Add non-simulation game logic and AI code here
}


void App::onNetwork() {
    GApp::onNetwork();
    // Poll net messages here
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }

    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { 
        shared_ptr<DefaultRenderer> r = dynamic_pointer_cast<DefaultRenderer>(m_renderer);
        r->setDeferredShading(! r->deferredShading());
        return true; 
    }

    return false;
}


void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);
    (void)ui;
    // Add key handling here based on the keys currently held or
    // ones that changed in the last frame.
}


void App::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    GApp::onPose(surface, surface2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}


void App::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the constructor so that exceptions can be caught.
}
