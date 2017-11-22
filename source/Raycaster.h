#pragma once
#include <G3D/G3D.h>

class Raycaster {
	
	public:
		void RenderImage(shared_ptr<Camera> camera, shared_ptr<Scene> scene, shared_ptr<Image> & img, RaycasterOptions options);
		Color3 GetPixelColor(shared_ptr<Camera> camera, shared_ptr<Scene>  scene, int x, int y, int width, int height, bool fixedPrimitives, int indirectRays);
		shared_ptr<UniversalSurfel> CastSingleRay(shared_ptr<Scene> scene, Ray ray, bool fixedPrimitives);
		void GenerateRay(Ray * rayBuffer);
		void IntersectRay(Ray * rayBuffer, UniveralSurfel * surfelBuffer);
		void GetShadowRay(shared_ptr<Light> light, UniversalSurfel * surfelBuffer, Ray* shadowRayBuffer);
		void ShadowTest(Ray * shadowRayBuffer, UniversalSurfel * surfelBuffer, bool* lightShadowedBuffer);
		void ScatterRay(Ray* rayBuffer, UniversalSurfel * surfelBuffer);
};

struct RaycasterOptions {
	public:
		int numRays;
		int numScatteringEvents;
};