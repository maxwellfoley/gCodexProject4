#pragma once
#include <G3D/G3D.h>

class Raycaster {
	
public:
	void RenderImage(shared_ptr<Camera> camera, shared_ptr<Scene> scene, shared_ptr<Image> & img, bool fixedPrimitives, bool multithreading, int indirectRays);
	Color3 GetPixelColor(shared_ptr<Camera> camera, shared_ptr<Scene>  scene, int x, int y, int width, int height, bool fixedPrimitives, bool multithreading, int indirectRays);
	shared_ptr<UniversalSurfel> CastSingleRay(shared_ptr<Scene> scene, Ray ray);
	bool IntersectTriangle(Ray ray, Triangle t, float b[3], float& dist);

};