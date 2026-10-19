#ifndef GAME_CLIENT_COMPONENTS_RACETOOL_H
#define GAME_CLIENT_COMPONENTS_RACETOOL_H

#include <engine/console.h>
#include <game/client/component.h>
#include <game/client/prediction/entities/character.h>

class CRaceTool : public CComponent
{
	enum EMarkerType {
		ZONE,
		TIMECP,
	};

	class CMarker {
	public:
		EMarkerType m_Type = TIMECP;
		int m_TimeCp = 0;
		
		float m_PosX = 0.0f;
		float m_PosY = 0.0f;
		float m_SizeX = 0.0f;
		float m_SizeY = 0.0f;
		
		bool IsInside(int, CGameClient*);
		bool TileValid(int TileIndex, CGameClient *client);
		void Render(CGameClient* client, ColorRGBA color);
	};
	
	bool m_InsideMarker = false;
	CMarker m_MarkerStart;
	CMarker m_MarkerEnd;
	int m_TimerStartTick = 0;
	
	CMarker *GetMarker(const char *pType);
public:
	CRaceTool();
	int Sizeof() const override { return sizeof(*this); }
	
	void OnConsoleInit() override;
	void OnNewSnapshot() override;
	void OnRender() override;
	static void Con_RTSetMarkerTimeCp(IConsole::IResult *pResult, void *pUserData);
	static void Con_RTSetMarkerPos(IConsole::IResult *pResult, void *pUserData);
};
#endif
