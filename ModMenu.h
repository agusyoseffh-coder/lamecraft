#ifndef MODMENU_H
#define MODMENU_H

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

struct ModInfo
{
	std::string fileName, name, author;
	float jump, speed;
	bool fly;
};

struct ModState
{
	std::vector<ModInfo> list;
	ModInfo active;
	bool on;
	int pos;
	ModState() : on(false), pos(0) {}
};

inline ModState& Mods()
{
	static ModState s;
	return s;
}

inline bool ModLoad(const std::string& path, ModInfo& m)
{
	FILE* fp = fopen(path.c_str(), "r");
	if(!fp) return false;
	char line[128];
	if(!fgets(line, sizeof(line), fp) || strncmp(line, "PM1", 3) != 0)
	{ fclose(fp); return false; }

	m.fileName = path; m.name = "Mod"; m.author = "?";
	m.jump = 1.0f; m.speed = 1.0f; m.fly = false;

	while(fgets(line, sizeof(line), fp))
	{
		char* eq = strchr(line, '=');
		if(!eq) continue;
		*eq = 0;
		char* v = eq + 1;
		v[strcspn(v, "\r\n")] = 0;
		if(!strcmp(line, "name")) m.name = v;
		else if(!strcmp(line, "author")) m.author = v;
		else if(!strcmp(line, "jump")) m.jump = (float)atof(v);
		else if(!strcmp(line, "speed")) m.speed = (float)atof(v);
		else if(!strcmp(line, "fly")) m.fly = (atoi(v) == 1);
	}
	fclose(fp);
	return true;
}

inline void ModScan()
{
	ModState& s = Mods();
	s.list.clear();
	s.pos = 0;
	DIR* d = opendir("Mods/");
	if(!d) return;
	struct dirent* e;
	while((e = readdir(d)) != NULL)
	{
		std::string f = e->d_name;
		if(f.size() > 3 && f.substr(f.size() - 3) == ".pm")
		{
			ModInfo m;
			if(ModLoad("Mods/" + f, m)) s.list.push_back(m);
		}
	}
	closedir(d);
}

// return true kalau user menekan tombol kembali
inline bool ModHandle(SystemManager* sys, SoundManager* snd)
{
	ModState& s = Mods();
	int n = (int)s.list.size();

	if(n > 0 && sys->KeyPressed(PSP_CTRL_UP))
	{ s.pos--; if(s.pos < 0) s.pos = n - 1; snd->PlayMenuSound(); }
	if(n > 0 && sys->KeyPressed(PSP_CTRL_DOWN))
	{ s.pos++; if(s.pos >= n) s.pos = 0; snd->PlayMenuSound(); }

	if(n > 0 && sys->KeyPressed(PSP_CTRL_CROSS))
	{
		if(s.on && s.active.fileName == s.list[s.pos].fileName)
			s.on = false;
		else
		{ s.active = s.list[s.pos]; s.on = true; }
	}
	return sys->KeyPressed(PSP_CTRL_CIRCLE);
}

inline void ModDraw(RenderManager* r, Sprite* back)
{
	ModState& s = Mods();

	sceGuDisable(GU_DEPTH_TEST);
	sceGuEnable(GU_BLEND);
	sceGuColor(GU_COLOR(1,1,1,1.0f));
	for(int x = 0; x < 16; x++)
		for(int y = 0; y < 9; y++)
		{ back->SetPosition(x*32, y*32); back->Draw(); }
	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_DEPTH_TEST);

	r->SetFontStyle(0.7f, 0xFFFFFFFF, 0xFF000000, 0x00000000);
	r->DebugPrint(40, 30, "MODS (.pm)");

	if(s.list.empty())
		r->DebugPrint(40, 100, "Tidak ada mod .pm di folder Mods/");

	int start = (s.pos / 6) * 6;
	for(int i = start; i < (int)s.list.size() && i < start + 6; i++)
	{
		bool isOn = s.on && s.active.fileName == s.list[i].fileName;
		r->SetFontStyle(i == s.pos ? 0.8f : 0.6f,
						i == s.pos ? 0xFF00FFFF : 0xFFFFFFFF,
						0xFF000000, 0x00000000);
		r->DebugPrint(40, 70 + (i - start) * 25, "%s %s - %s",
					  isOn ? "[ON] " : "[OFF]",
					  s.list[i].name.c_str(), s.list[i].author.c_str());
	}
	r->SetFontStyle(0.5f, 0xFFFFFFFF, 0xFF000000, 0x00000000);
	r->DebugPrint(40, 255, "X: ON/OFF   O: Kembali");
}

#endif
