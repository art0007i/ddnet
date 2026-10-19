
#include "racetool.h"

#include <base/process.h>
#include <base/str.h>
#include <base/time.h>

#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/gameclient.h>
#include <game/client/race.h>
#include <game/localization.h>

CRaceTool::CRaceTool() {
}

void CRaceTool::OnConsoleInit()
{
	Console()->Register("rt_set_marker_zone", "s[type] f[x] f[y] f[w] f[h]", CFGFLAG_CLIENT, Con_RTSetMarkerPos, this, "Set marker to zone. Type can be start or end");
	Console()->Register("rt_set_marker_timecp", "s[type] i[timecp]", CFGFLAG_CLIENT, Con_RTSetMarkerTimeCp, this, "Set marker to timecp (put 33 for startline, 34 for finish). Type can be start or end");	
}

CRaceTool::CMarker *CRaceTool::GetMarker(const char *pType)
{
	if(!strcmp(pType, "start"))
		return &m_MarkerStart;
	
	if(!strcmp(pType, "end"))
		return &m_MarkerEnd;
	
	return nullptr;
}

void CRaceTool::Con_RTSetMarkerPos(IConsole::IResult *pResult, void *pUserData)
{
	CRaceTool *pSelf = (CRaceTool *)pUserData;
	const char *pType = pResult->GetString(0);
	CMarker* mark = pSelf->GetMarker(pType);
	if(mark == nullptr) {
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", "Invalid marker. must be start or end");
		return;
	}
	
	const float x = pResult->GetFloat(1);
	const float y = pResult->GetFloat(2);
	const float w = pResult->GetFloat(3);
	const float h = pResult->GetFloat(4);
	mark->m_PosX = x;
	mark->m_PosY = y;
	mark->m_SizeX = w;
	mark->m_SizeY = h;
	mark->m_Type = EMarkerType::ZONE;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "updated marker %s to Zone (x=%f, y=%f, w=%f, h=%f)",
		pType, x,y,w,h);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", aBuf);
}

void CRaceTool::Con_RTSetMarkerTimeCp(IConsole::IResult *pResult, void *pUserData)
{
	CRaceTool *pSelf = (CRaceTool *)pUserData;
	const char *pType = pResult->GetString(0);
	CMarker* mark = pSelf->GetMarker(pType);
	if(mark == nullptr) {
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", "Invalid marker. must be start or end");
		return;
	}
	
	const int cp = pResult->GetInteger(1);
	mark->m_TimeCp = cp;
	mark->m_Type = EMarkerType::TIMECP;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "updated marker %s to TimeCp %d",
		pType, cp);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", aBuf);
}

bool CRaceTool::CMarker::TileValid(int TileIndex, CGameClient* client) {
	auto cp = client->Collision()->IsTimeCheckpoint(TileIndex);
	auto cpFront = client->Collision()->IsFrontTimeCheckpoint(TileIndex);
	
	
	auto norm = client->Collision()->GetTileIndex(TileIndex);
	auto front = client->Collision()->GetFrontTileIndex(TileIndex);
	
	// TILE_START = 33, TILE_FINISH = 34
	if (m_TimeCp == TILE_START || m_TimeCp == TILE_FINISH)
		if(m_TimeCp == norm || m_TimeCp == front)
			return true;
	
	if (cp > -1 && cp == m_TimeCp-1) {
		return true;
	}
	
	if (cpFront > -1 && cpFront == m_TimeCp-1) {
		return true;
	}
	
	return false;
}

bool CRaceTool::CMarker::IsInside(int clientId, CGameClient* client) {
	auto character = client->m_Snap.m_aCharacters[clientId];
	
	auto pCurChar = character.m_Cur;
	auto pPrevChar = character.m_Prev;
	const float IntraTick = client->Client()->IntraGameTick(g_Config.m_ClDummy);
	
	// To make the player position relative to blocks we need to divide by the block size
	auto mixPos = mix(vec2(pPrevChar.m_X, pPrevChar.m_Y), vec2(pCurChar.m_X, pCurChar.m_Y), IntraTick) / 32.0f;
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "chk! type = %d, curX = %f, pX = %f, sX = %f",
		m_Type, mixPos.x, m_PosX, m_SizeX);
	client->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "racetool", aBuf);

	if (m_Type == EMarkerType::ZONE) {
		if (mixPos.x < m_PosX-(m_SizeX/2) || mixPos.x > m_PosX+(m_SizeX/2)) return false;
		if (mixPos.y < m_PosY-(m_SizeY/2) || mixPos.y > m_PosY+(m_SizeY/2)) return false;
		return true;
	}
	if (m_Type == EMarkerType::TIMECP) {
		auto prevPos = vec2(pPrevChar.m_X, pPrevChar.m_Y);
		auto curPos = vec2(pCurChar.m_X, pCurChar.m_Y);
		int CurrentIndex = client->Collision()->GetMapIndex(curPos);
		std::vector<int> vIndices = client->Collision()->GetMapIndices(prevPos, curPos);
		if(!vIndices.empty())
			for(int Index : vIndices) {
				if(TileValid(Index, client))
					return true;
			}
		else
		{
			return TileValid(CurrentIndex, client);
		}
	}
	return false;
}

void CRaceTool::OnNewSnapshot()
{
	// TODO: raceconfig enable
	auto m_RaceToolEnable = true;
	
	if(!GameClient()->m_GameInfo.m_Race || !m_RaceToolEnable || Client()->State() != IClient::STATE_ONLINE)
		return;
	
	if(!GameClient()->m_Snap.m_pGameInfoObj || !GameClient()->m_Snap.m_pLocalCharacter || !GameClient()->m_Snap.m_pLocalPrevCharacter)
		return;
	
	int RaceTick = Client()->GameTick(g_Config.m_ClDummy);
	int localId = GameClient()->m_Snap.m_LocalClientId;
	
	bool isInsideStart = m_MarkerStart.IsInside(localId, GameClient());
	if(isInsideStart && !m_InsideMarker) {
		m_TimerStartTick = RaceTick;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "inside start marker! at tick -> %d",
			RaceTick);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", aBuf);
	}
	
	bool isInsideEnd = m_MarkerEnd.IsInside(localId, GameClient());
	if(isInsideEnd && !m_InsideMarker) {
		auto delta = RaceTick - m_TimerStartTick;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "inside end marker! at tick -> %d, delta = %d",
			RaceTick, delta);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", aBuf);
		
		float Time = delta / (float)Client()->GameTickSpeed();
		str_format(aBuf, sizeof(aBuf),
			"Segment time: %.2f",
			Time);
		auto oldColor = g_Config.m_ClMessageClientColor;
		g_Config.m_ClMessageClientColor = 3407744; // #CCFF01
		GameClient()->Echo(aBuf);
		g_Config.m_ClMessageClientColor = oldColor;
	}
	
	m_InsideMarker = isInsideStart || isInsideEnd;
}


void CRaceTool::OnRender()
{	
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	/*
	float MinX, MinY, MaxX, MaxY;	
	Graphics()->GetScreen(&MinX, &MinY, &MaxX, &MaxY);

	int localId = GameClient()->m_Snap.m_LocalClientId;
	auto character = GameClient()->m_Snap.m_aCharacters[localId];
	auto pCurChar = character.m_Cur;
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "rect: min(%f, %f) max(%f, %f) px = %d",
		MinX, MinY, MaxX, MaxY, pCurChar.m_X);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "racetool", aBuf);
	*/
	
	m_MarkerStart.Render(GameClient(), ColorRGBA(0.2, 0.8, 0.2, 0.5));
	m_MarkerEnd.Render(GameClient(), ColorRGBA(0.8, 0.2, 0.2, 0.5));
}

void CRaceTool::CMarker::Render(CGameClient* client, ColorRGBA color) {
	if (m_Type == EMarkerType::ZONE) {
		vec2 Center = client->m_Camera.m_Center;
		float Zoom = client->m_Camera.m_Zoom;
		float radius = 4 / Zoom;
		
		vec2 size = vec2(m_SizeX, m_SizeY);
		vec2 topCorner = vec2(m_PosX, m_PosY) - (size / 2);
		
		float aPoints[4];
		client->Graphics()->MapScreenToWorld(Center.x, Center.y, 100, 100, 100, 0, 0, client->Graphics()->ScreenAspect(), Zoom, aPoints);
		client->Graphics()->MapScreen(aPoints[0], aPoints[1], aPoints[2], aPoints[3]);
		client->Graphics()->DrawRect(topCorner.x * 32, topCorner.y * 32, size.x * 32, size.y * 32, color, IGraphics::CORNER_ALL, radius);
		/*
		client->Graphics()->MapScreen(0,0, 32, 32);
		
		float MinX, MinY, MaxX, MaxY;
		client->Graphics()->GetScreen(&MinX, &MinY, &MaxX, &MaxY);
		vec2 ScreenCenter = vec2(MaxX - MinX, MaxY - MinY) / 2;
		
		vec2 Center = client->m_Camera.m_Center;
		float Zoom = client->m_Camera.m_Zoom;
		
		vec2 ZonePos = ((vec2(m_PosX, m_PosY) * 32) / Zoom) - Center + ScreenCenter;
		vec2 ZoneSize = (vec2(m_SizeX, m_SizeY) * 32) / Zoom;
		
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "rect: center(%f, %f) zoom(%f) min(%f, %f) max(%f, %f) ZP(%f, %f) ZS(%f, %f) final(%f, %f, %f, %f)",
			Center.x, Center.y, Zoom, MinX, MinY, MaxX, MaxY, m_PosX, m_PosY, m_SizeX, m_SizeY, ZonePos.x, ZonePos.y, ZoneSize.x , ZoneSize.y);
		client->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "racetool", aBuf);
		
		
		client->Graphics()->DrawRect(ZonePos.x, ZonePos.y, ZoneSize.x , ZoneSize.y, color, IGraphics::CORNER_ALL, radius);
		client->Graphics()->DrawRect(50 , 50, 50, 50, color, IGraphics::CORNER_ALL, radius);
		*/
	}
}
