#include "Raycaster.h"

TriTree tt;
shared_ptr<Image> & img;
shared_ptr<Camera> camera;
void Raycaster::RenderImage(shared_ptr<Camera> _camera, shared_ptr<Scene> scene, shared_ptr<Image> & __img, RaycasterOptions options)
{
	img = __img;
	camera = _camera;
	
	//get all triangles and etc. in the scene	
	Array<shared_ptr<Surface> > surfaceArray = Array<shared_ptr<Surface> >();
	scene->onPose(surfaceArray);
	tt.setContents(surfaceArray);
	
	//get all lights in scene
	Array<shared_ptr<Light>> lights;
	scene->getTypedEntityArray(lights);

	Color3 modulationBuffer[img->width()*img->height()];
	Color3 colorBuffer[img->width()*img->height()];

	Ray rayBuffer[img->width()*img->height()];
	UniversalSurfel surfelBuffer[img->width()*img->height()];
	Biradiance3 biradianceBuffer[img->width()*img->height()];
	Ray shadowRayBuffer[img->width()*img->height()];
	bool lightShadowedBuffer[img->width()*img->height()];
	
	
	//G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	//[this,_img = &img, _camera = &camera, _scene = &scene, fp = &fixedPrimitives, ir = &indirectRays](Point2int32 p){
		
		//Color3 color; 
		for(int i = 0; i < options.numRays; i++)
		{
			//generate primary rays 
			Raycaster::GenerateRay(rayBuffer);
			
			//initialize the modulationBuffer to Color3(1 / paths per pixel)
			G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
			[this,_img = &img](Point2int32 p){
				modulationBuffer[img->width()*p.y + p.x] = Color3(1/numRays);
				colorBuffer[img->width()*p.y + p.x] = Color3(0);
			});
			
			//repeat for the number of scattering events:
			for(int j = 0; j < options.numScatteringEvents; j++)
			{
				//intersect all rays, storing the result in surfelbuffer
				Raycaster::IntersectRay(rayBuffer, surfelBuffer);
				
				//add emissive terms
				G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
				[this,_img = &img](Point2int32 p){
					colorBuffer[img->width()*p.y + p.x] += surfelBuffer[img->width()*p.y + p.x]->emittedRadiance()*modulationBuffer[img->width()*p.y + p.x];
				});
				
				
				//if there are lights, do lights
				//TODO: eventually change this to importance sampling 
				if(lights.size() > 0)
				{
					int k = randint(0, lights.size()-1);
					//get ray for shadow calculation
					Raycaster::GetShadowRay(lights[k], surfelBuffer, shadowRayBuffer);
					
					//figure out whether or not the surfel is in shadow
					Raycaster::ShadowTest(shadowRayBuffer, surfelBuffer, lightShadowedBuffer);
						 
					G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
					[this,_img = &img](Point2int32 p){
						if(!lightShadowedBuffer[img->width()*p.y + p.x])
						{
							float multiplier = lightDir.dot(surfel->geometricNormal);

							//shade the pixel
							Color3 addTo = surfel->lambertianReflectivity * multiplier * lights[k]->biradiance(surfel->position);
							if(addTo.r < 0.0){ addTo.r = 0.0;}
							if(addTo.g < 0.0){ addTo.g = 0.0;}
							if(addTo.b < 0.0){ addTo.b = 0.0;}
							
							colorBuffer[img->width()*p.y + p.x] += addTo*modulationBuffer[img->width()*p.y + p.x];
						}
					});
				}
				//if not the last iteration, scatter the rays 
				if(j < options.numScatteringEvents-1)
				{
					Raycaster::ScatterRay(rayBuffer, surfelBuffer, modulationBuffer);
				}
			}
		}


	debugPrintf("process done \n");
	
}

void Raycaster::GenerateRay(Ray * rayBuffer)
{

	G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	[this,_img = &img, _camera = &camera](Point2int32 p){
		
		//calculate ray direction and origin based on camera position
		const float side = -2.0f * tan(	camera->fieldOfViewAngle ()  / 2.0f);

		float _x, _y, _z, _ya, _p, _r;
		camera->frame().getXYZYPRDegrees(_x, _y, _z, _ya, _p, _r);

		Point3 p = Point3( camera->nearPlaneZ() * (float(p.x) / float(img->width()) - 0.5f) * side * float(img->width()) / float(img->height()) , 
		                camera->nearPlaneZ() * -(float(p.y) / float(img->height()) - 0.5f) * side, 
		                camera->nearPlaneZ())  + Point3(_x,_y,_z);
		Vector3 dir = p-Point3(_x,_y,_z);
		dir = dir.unit();	
 		rayBuffer[img->width()*p.y + p.x] = Ray(p,dir);

	});
}

void Raycaster::IntersectRay(Ray * rayBuffer, UniversalSurfel * surfelBuffer)
{
	G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	[](Point2int32 p){

		Ray ray = rayBuffer[img->width()*p.y + p.x];

		Point3 P = ray.origin();
		Vector3 w = ray.direction();
		 //debugPrintf("ray point %f %f %f \n",P.x,P.y,P.z);
		//debugPrintf("ray direction %f %f %f \n",w.x,w.y,w.z);

		TriTreeBase::Hit hit;
		bool foundHit = tt.intersectRay(ray, hit);

		//if it hit something
		if(foundHit)
		{
				lazy_ptr<UniversalMaterial> material = tt[hit.triIndex].material();

				
				shared_ptr<UniversalSurfel> surfel = std::make_shared<UniversalSurfel>(tt[hit.triIndex],hit.u,hit.v,hit.triIndex,tt.vertexArray(),false);
				surfel->sample(tt[hit.triIndex],hit.u,hit.v,hit.triIndex,tt.vertexArray(),false,material.resolve().get());
				surfel->position = P+w*hit.distance;

				surfelBuffer[img->width()*p.y + p.x] = surfel;
			
		}
		else
		{
			surfelBuffer[img->width()*p.y + p.x] = shared_ptr<UniversalSurfel>();
		}
	});
	
}

void Raycaster::GetShadowRay(shared_ptr<Light> light, UniversalSurfel * surfelBuffer, Ray* shadowRayBuffer)
{
	G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	[](Point2int32 p){
		
		Vector3 pos = light->position().xyz();
		Vector3 lightDir = pos - surfelBuffer[img->width()*p.y + p.x]->position;
		lightDir = lightDir.unit();
		shadowRayBuffer[img->width()*p.y + p.x] = Ray(pos, -lightDir);
	});
	
}

void Raycaster::ShadowTest(Ray * shadowRayBuffer, UniversalSurfel * surfelBuffer, bool* lightShadowedBuffer)
{
	G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	[](Point2int32 p){
		
		TriTreeBase::Hit hit;
		bool foundHit = tt.intersectRay(shadowRayBuffer[img->width()*p.y + p.x], hit);
		if(foundHit && hit.distance < Vector3.magnitude(shadowRayBuffer[img->width()*p.y + p.x] - surfelBuffer[img->width()*p.y + p.x]->position))
		{
			lightShadowedBuffer[img->width()*p.y + p.x] = true;
		}
		else
		{
			lightShadowedBuffer[img->width()*p.y + p.x] = false;
		}

	});
}

void Raycaster::ScatterRay(Ray * rayBuffer, UniversalSurfel * surfelBuffer, Color3 * modulationBuffer)
{
	G3D::runConcurrently(Point2int32(0, 0), Point2int32(img->width(), img->height()), 
	[](Point2int32 p){
		shared_ptr<UniversalSurfel> surfel = surfelBuffer[img->width()*p.y + p.x];

		Vector3 w_before = rayBuffer[img->width()*p.y + p.x].direction();
		Random rng =  G3D::Random::threadCommon();
		Vector3 w_after;
		Color3 weight; 
		
		surfel.scatter(PathDirection::EYE_TO_SOURCE, w_before, false, rng, weight, w_after);
		modulationBuffer[img->width()*p.y + p.x]*=weight;
		
		rayBuffer[img->width()*p.y + p.x] = Ray(surfel->position, w_after);
	});

}

Color3 Raycaster::GetPixelColor(shared_ptr<Camera> camera, shared_ptr<Scene>  scene, int x, int y, int width, int height, bool fixedPrimitives, int indirectRays)
{			
	/*
	//calculate ray direction and origin based on camera position
  const float side = -2.0f * tan(	camera->fieldOfViewAngle ()  / 2.0f);
  float _x, _y, _z, _ya, _p, _r;
  camera->frame().getXYZYPRDegrees(_x, _y, _z, _ya, _p, _r);

	Point3 p = Point3( camera->nearPlaneZ() * (float(x) / float(width) - 0.5f) * side * float(width) / float(height) , 
	                camera->nearPlaneZ() * -(float(y) / float(height) - 0.5f) * side, 
	                camera->nearPlaneZ())  + Point3(_x,_y,_z);
	Vector3 dir = p-Point3(_x,_y,_z);
	dir = dir.unit();	
	Ray ray = Ray(p,dir);
	
	*/
	/*
	debugPrintf("\n\nCalculating pixel %i, %i\n",x,y);
	shared_ptr<UniversalSurfel> surfel = CastSingleRay(scene, ray, fixedPrimitives);
	
*/
	//simply tell if it hits something 
	if(notNull(surfel))
	{
		//lambertian
		Color3 lamb =  surfel->lambertianReflectivity; 
		Color3 color;
		if(indirectRays==0) {color = lamb*.2;}
		debugPrintf("surfel position after %f %f %f \n" ,surfel->position.x,surfel->position.y,surfel->position.z);
		
		//get all lights in scene
		Array<shared_ptr<Light>> lights;
		scene->getTypedEntityArray(lights);
		
		//loop through
		//for all lights
		for(int i = 0; i < lights.size(); i++)
		{
				// multiply by dot product of light ray and normal
				Vector3 pos = lights[i]->position().xyz();
				Vector3 lightDir = pos - surfel->position;
				lightDir = lightDir.unit();
				
				bool shadowed = false;
				
				if(lights[i]->shadowsEnabled())
				{
					//test for shadow, cast ray from light
					Ray shadowTestRay = Ray(pos, -lightDir);
					shared_ptr<UniversalSurfel> shadowTestSurfel = CastSingleRay(scene, shadowTestRay, fixedPrimitives);
					
					
					if(notNull(shadowTestSurfel) && abs((pos - shadowTestSurfel->position).magnitude())+.1 < abs(pos-surfel->position).magnitude())
					{
						shadowed = true;
					}
				}
				if(!shadowed)
				{
					//debugPrintf("lightdir %f %f %f \n", lightDir.x, lightDir.y, lightDir.z);
					//debugPrintf("surfel normal %f %f %f \n", surfel->geometricNormal.x, surfel->geometricNormal.y, surfel->geometricNormal.z);

					float multiplier = lightDir.dot(surfel->geometricNormal);
					debugPrintf("multiplier %f \n",multiplier);
					Color3 addTo = lamb * multiplier * lights[i]->biradiance(surfel->position);
					if(addTo.r < 0.0){ addTo.r = 0.0;}
					if(addTo.g < 0.0){ addTo.g = 0.0;}
					if(addTo.b < 0.0){ addTo.b = 0.0;}
					color += addTo;
			}
		}		
		//ambient lighting
		for(int i = 0; i < indirectRays; i++)
		{
			Vector3 indirectLightDir = Vector3::hemiRandom(surfel->geometricNormal);
			Ray indirectLightRay = Ray(surfel->position, indirectLightDir.unit());
			Color3 otherLamb = Color3(0.0,0.0,0.0);
			shared_ptr<UniversalSurfel> indirectSurfel = CastSingleRay(scene,indirectLightRay, fixedPrimitives);
			if(notNull(indirectSurfel)) {otherLamb = indirectSurfel->lambertianReflectivity; } 
			float multiplier = indirectLightDir.dot(surfel->geometricNormal);

			Color3 addTo = otherLamb * multiplier / indirectRays;
			if(addTo.r < 0.0){ addTo.r = 0.0;}
			if(addTo.g < 0.0){ addTo.g = 0.0;}
			if(addTo.b < 0.0){ addTo.b = 0.0;}
			color += addTo;

		}
		return color;
	}
	else {
		debugPrintf("not found %i, %i\n",x,y);

		return Color3(0.,0.,0.);	
	}

	
}

shared_ptr<UniversalSurfel> Raycaster::CastSingleRay(shared_ptr<Scene> scene, Ray ray, bool fixedPrimitives)
{
	


	Point3 P = ray.origin();
	Vector3 w = ray.direction();
	 //debugPrintf("ray point %f %f %f \n",P.x,P.y,P.z);
	//debugPrintf("ray direction %f %f %f \n",w.x,w.y,w.z);

	TriTreeBase::Hit hit;
	bool foundHit = tt.intersectRay(ray, hit);

	//if it hit something
	if(foundHit)
	{
			lazy_ptr<UniversalMaterial> material = tt[hit.triIndex].material();

			
			shared_ptr<UniversalSurfel> surfel = std::make_shared<UniversalSurfel>(tt[hit.triIndex],hit.u,hit.v,hit.triIndex,tt.vertexArray(),false);
			surfel->sample(tt[hit.triIndex],hit.u,hit.v,hit.triIndex,tt.vertexArray(),false,material.resolve().get());
			surfel->position = P+w*hit.distance;

			return surfel;
		
	}
	else
	{
		return shared_ptr<UniversalSurfel>();
	}
}	
