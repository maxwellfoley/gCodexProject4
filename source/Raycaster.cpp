#include "Raycaster.h"


void Raycaster::RenderImage(shared_ptr<Camera> camera, shared_ptr<Scene> scene, shared_ptr<Image> & img, bool fixedPrimitives, bool multithreading, int indirectRays)
{
	for(int j = 0; j < img->height(); j++)
	{
		for(int i = 0; i < img->width(); i++)
		{	
			Color3 color = GetPixelColor(camera, scene, i, j, img->width(), img->height(), fixedPrimitives, multithreading, indirectRays);
			img->set(i, j, color);
		}
	}
	debugPrintf("process done \n");
	
}

Color3 Raycaster::GetPixelColor(shared_ptr<Camera> camera, shared_ptr<Scene>  scene, int x, int y, int width, int height, bool fixedPrimitives, bool multithreading, int indirectRays)
{			
	
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
	
	
	debugPrintf("\n\nCalculating pixel %i, %i\n",x,y);
	shared_ptr<UniversalSurfel> surfel = CastSingleRay(scene, ray );
	

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
					shared_ptr<UniversalSurfel> shadowTestSurfel = CastSingleRay(scene, shadowTestRay);
					
					
					if(notNull(shadowTestSurfel) && abs((pos - shadowTestSurfel->position).magnitude())+.1 < abs(pos-surfel->position).magnitude())
					{
						shadowed = true;
					}
				}
				if(!shadowed)
				{
					debugPrintf("lightdir %f %f %f \n", lightDir.x, lightDir.y, lightDir.z);
					debugPrintf("surfel normal %f %f %f \n", surfel->geometricNormal.x, surfel->geometricNormal.y, surfel->geometricNormal.z);

					float multiplier = lightDir.dot(surfel->geometricNormal);
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
			shared_ptr<UniversalSurfel> indirectSurfel = CastSingleRay(scene,indirectLightRay);
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

shared_ptr<UniversalSurfel> Raycaster::CastSingleRay(shared_ptr<Scene> scene, Ray ray)
{
	
	//get all triangles and etc. in the scene
	TriTree tt = TriTree();
	
	Array<shared_ptr<Surface> > surfaceArray = Array<shared_ptr<Surface> >();
	scene->onPose(surfaceArray);
	tt.setContents(surfaceArray);

	Point3 P = ray.origin();
	Vector3 w = ray.direction();
	debugPrintf("ray point %f %f %f \n",P.x,P.y,P.z);
	debugPrintf("ray direction %f %f %f \n",w.x,w.y,w.z);

	//find the closest triangle in the scene
	bool foundHit = false;
	float * minB;
	float minT = 10000000;
	int minI = -1;
	
	for(int i = 0; i < tt.size() ; i++)
	{
		float b [3];
		float t = 10000000;
		
		//MY VERSION 
		foundHit = IntersectTriangle(ray, tt[i].toTriangle(tt.vertexArray()),b,t);
		//G3D VERSION 
		//foundHit = tt[i].toTriangle(tt.vertexArray()).intersect(ray,t,b);

		if(t < minT && t >= 0.0f) { minB = b; minT = t; minI = i;}
	}
	
	//if it hit something
	if(minI > -1)
	{
		debugPrintf("Triangle index %i\n",minI);
		debugPrintf("barycenters %f %f %f\n",minB[0],minB[1],minB[2]);
		debugPrintf("distance %f \n",minT);

		//all of this just to get the surfel information
		TriTreeBase::Hit hit = TriTreeBase::Hit();
		
		hit.triIndex = minI;
		hit.distance = minT;
		
		Point2 texCoord =  Point2(0.0f,0.0f); 
		Point2 triUVs[3] ;

		for(int j=0;j <3; j++)
		{
			Point2 equivUV = tt.vertexArray().vertex[tt[minI].index[j]].texCoord0;
			triUVs[j] = tt[minI].texCoord(tt.vertexArray(), j);
			texCoord += minB[j]*triUVs[j];
		}
		
		hit.u = texCoord.x;
		hit.v = texCoord.y;
		
		lazy_ptr<UniversalMaterial> material = tt[minI].material();

		
		shared_ptr<UniversalSurfel> surfel = std::make_shared<UniversalSurfel>(tt[minI],hit.u,hit.v,minI,tt.vertexArray(),false);
		surfel->sample(tt[minI],hit.u,hit.v,minI,tt.vertexArray(),false,material.resolve().get());
		surfel->position = P+w*minT;

		return surfel;
	}
	else
	{
		return shared_ptr<UniversalSurfel>();
	}
}

bool Raycaster::IntersectTriangle(Ray ray, Triangle t, float b[3], float& dist)
{
	Point3 P = ray.origin();
	Vector3 w = ray.direction();

	//edge vectors
	const Vector3& e_1 = t.vertex(1) - t.vertex(0);
	const Vector3& e_2 = t.vertex(2) - t.vertex(0);

	// Face normal
	const Vector3& n = e_1.cross(e_2).direction();

	const Vector3& q = w.cross(e_2);
	const float a = e_1.dot(q);

	// Backfacing / nearly parallel, or close to the limit of precision?
 	if ((n.dot(w) >= 0) || (abs(a) <= .0000001)) return false;
	
	const Vector3& s = (P - t.vertex(0)) / a;
	const Vector3& r = s.cross(e_1);
	
  b[0] = s.dot(q);
	b[1] = r.dot(w);
	b[2] = 1.0f - b[0] - b[1];

	// Intersected outside triangle?
	if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) return false;

  dist = e_2.dot(r);
	return (dist >= 0.0f);
}
