/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_NAMEPLATES_H
#define GAME_CLIENT_COMPONENTS_NAMEPLATES_H
#include <base/vmath.h>

#include <engine/shared/protocol.h>
#include <engine/textrender.h>

#include <game/client/component.h>

struct CNetObj_Character;
struct CNetObj_PlayerInfo;

struct SPlayerNamePlate
{
	SPlayerNamePlate()
	{
		Reset();
	}

	void Reset()
	{
		m_NameTextContainerIndex.Reset();
		m_ClanNameTextContainerIndex.Reset();
		m_SkinNameTextContainerIndex.Reset();
		m_aName[0] = 0;
		m_aClanName[0] = 0;
		m_aSkinName[0] = 0;
		m_NameTextWidth = m_ClanNameTextWidth = m_SkinNameTextWidth = 0.f;
		m_NameTextFontSize = m_ClanNameTextFontSize = m_SkinNameTextFontSize = 0;
	}

	char m_aName[MAX_NAME_LENGTH];
	float m_NameTextWidth;
	STextContainerIndex m_NameTextContainerIndex;
	float m_NameTextFontSize;

	char m_aClanName[MAX_CLAN_LENGTH];
	float m_ClanNameTextWidth;
	STextContainerIndex m_ClanNameTextContainerIndex;
	float m_ClanNameTextFontSize;

	char m_aSkinName[64];
	float m_SkinNameTextWidth;
	STextContainerIndex m_SkinNameTextContainerIndex;
	float m_SkinNameTextFontSize;
};

class CNamePlates : public CComponent
{
	void RenderNameplate(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPlayerInfo);
	void RenderNameplatePos(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha = false);

	SPlayerNamePlate m_aNamePlates[MAX_CLIENTS];

	void ResetNamePlates();

	int m_DirectionQuadContainerIndex;

public:
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnWindowResize() override;
	virtual void OnInit() override;
	virtual void OnRender() override;
};

#endif
