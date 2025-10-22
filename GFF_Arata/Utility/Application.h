#pragma once
#include "ProjectConfig.h"
#include "Singleton.h"
#include <string>
#include "../Scenes/SceneManager.h"

#define D_SUCCESS		(0)		//ê¨å˜
#define D_FAILURE		(-1)	//é∏îs

class Application : public Singleton<Application>
{
private:
	float delta_second;
	LONGLONG start_time;
	LONGLONG now_time;
	float refresh_rate;
	class SceneManager* scene;
	bool exit;						//èIóπÉtÉâÉO

public:
	Application();
	~Application();
	bool WakeUp();
	void Run();
	void Shutdown();
	int ErrorThrow(std::string error_log);
	float Get_delta_Second();
private:
	void UpdateDeLtaTime();
};

