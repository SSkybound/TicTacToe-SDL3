#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <string>
#include <string_view>
#include <array>

enum class BoardState
{
	// Unselected tiles
	VOID_STATE,

	// Selected tiles
	X_STATE,
	O_STATE,
};

struct V2
{
	int x{};
	int y{};
};

class Game
{
public:

	// Constructor opens window & sets properties
	Game(std::string_view title)
		: title{ title }
	{
		m_pWindow = SDL_CreateWindow(title.data(), m_width, m_height, SDL_WINDOW_RESIZABLE);
		SDL_SetWindowMinimumSize(m_pWindow, m_width, m_height);
	}

	~Game()
	{
		// Audio destruction
		MIX_DestroyAudio(m_pInvalidSelection);
		MIX_DestroyTrack(m_pSfxTrack);
		MIX_DestroyMixer(m_pMixer);
		MIX_Quit();

		// Graphics destruction
		SDL_DestroyTexture(m_pBoardTexture);
		SDL_DestroyTexture(m_pXOTexture);
		SDL_DestroyRenderer(m_pRenderer);
		SDL_DestroyWindow(m_pWindow);

		SDL_Quit();
	}

	void Init();

private:

	// Main application loop
	void Run();

	// Primary loop functions
	void Input();
	void Update();
	void Render();

	void RenderText();
	
	// Board rendering
	void RenderBoardState();
	void RenderXTexture(const int index);
	void RenderOTexture(const int index);

	// Input handling
	void MouseEvent(SDL_Event &e);
	void KeyboardEvent(SDL_Event& e);
	void PlaceOnGrid(const int index);

	// Audio streaming
	void PlayInvalidSfx();

	// XO Sprite Selection
	V2 GetFromSpriteSheet(const int x, const int y);

	// Called after each player turn
	void CheckWinConditions();

	// Called upon win condition
	void GameFinished(BoardState& winner);

	// Tracks status of each grid space
	std::array<BoardState, 9> m_boardStates
	{
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
	};

	static constexpr std::array<BoardState, 9> m_initalBoardStates
	{
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
		BoardState::VOID_STATE, BoardState::VOID_STATE, BoardState::VOID_STATE,
	};
	
	// Where to draw textures & where player has clicked
	static constexpr std::array<SDL_FRect, 9> m_boardGrid
	{{
		{285, 75, 220, 180}, {510, 75, 280, 180}, {800, 75, 280, 180},
		{285, 260, 220, 180}, {510, 260, 280, 180}, {800, 260, 280, 180},
		{285, 475, 220, 180}, {510, 475, 280, 180}, {800, 470, 280, 180}
	}};

	
	// Application
	SDL_Window* m_pWindow{};
	SDL_Renderer* m_pRenderer{};

	// Textures
	SDL_Surface* m_pBoardSurface{};
	SDL_Texture* m_pBoardTexture{};
	SDL_Surface* m_pXOSurface{};
	SDL_Texture* m_pXOTexture{};

	// Window icon
	SDL_Surface* m_pWindowIcon{};

	// Master mix
	MIX_Mixer* m_pMixer{};
	
	// Sfx Audio
	MIX_Track* m_pSfxTrack{};
	MIX_Audio* m_pInvalidSelection{};

	// Terminates app on false
	bool m_windowRunning{ true };

	// Window specifications
	static constexpr int m_width{ 1280 };
	static constexpr int m_height{ 720 };

	// XO Textures (px)
	static constexpr int m_xoTextureSize{ 16 };

	// Timing
	Uint64 m_currentTime{};
	Uint64 m_lastTime{};
	Uint64 m_ticks{};

	// Title set by construction parameters
	std::string_view title{};

	// Player 1 turn on true, Player 2 turn on false
	bool m_p1Turn{ true };

	// Text to display when player attempts to select occupied tile
	bool m_invalidSelectionTextVisible{ false };
	Uint64 m_invalidSelectionStartTick{};

	// Tracks whether the game is over & the winner
	// VOID_STATE indicates a tie or ongoing game
	bool m_gameOver{ false };
	BoardState m_gameWinner{BoardState::VOID_STATE};
};

#endif