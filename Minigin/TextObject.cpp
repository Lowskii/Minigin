#include "MiniginPCH.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "TextObject.h"
#include "Renderer.h"
#include "Font.h"
#include "Texture2D.h"

minigin::TextObject::TextObject(const string& text, const shared_ptr<Font>& font)
	: m_NeedsUpdate(true), m_Text(text), m_Font(font), m_Texture(nullptr)
	, m_Color{ 255,255,255 }
{ }

void minigin::TextObject::Update()
{
	if (m_NeedsUpdate)
	{
		const auto surf = TTF_RenderText_Blended(m_Font->GetFont(), m_Text.c_str(), m_Color);
		if (surf == nullptr)
		{
			throw runtime_error(string("Render text failed: ") + SDL_GetError());
		}
		auto texture = SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf);
		if (texture == nullptr)
		{
			throw runtime_error(string("Create text texture from surface failed: ") + SDL_GetError());
		}
		SDL_FreeSurface(surf);
		m_Texture = make_shared<Texture2D>(texture);
		m_NeedsUpdate = false;
	}
}

void minigin::TextObject::Render() const
{
	if (m_Texture != nullptr && m_Visible)
	{
		const auto pos = m_Transform.GetPosition();
		Renderer::GetInstance().RenderTexture(*m_Texture, pos.x, pos.y);
	}
}

// This implementation uses the "dirty flag" pattern
void minigin::TextObject::SetText(const string& text)
{
	m_Text = text;
	m_NeedsUpdate = true;
}

void minigin::TextObject::SetPosition(const float x, const float y)
{
	m_Transform.SetPosition(x, y);
}

void minigin::TextObject::SetColor(SDL_Color color)
{
	m_Color = color;
}


