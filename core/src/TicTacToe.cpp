#include "TicTacToe.h"

#include <mdspan>


/*    Public - TicTacToe    */

void Game::Init()
{
	// Initalize dependencies
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	MIX_Init();

	// Set mixer to default audio device
	m_pMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

	// Create audio track for the master mixer
	m_pSfxTrack = MIX_CreateTrack(m_pMixer);

	// Background set to black
	SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);

	// Set renderer to default GPU device
	m_pRenderer = SDL_CreateGPURenderer(nullptr, m_pWindow);
	
	// Load assets into memory
	m_pBoardSurface = SDL_LoadPNG("assets/board.png");
	m_pXOSurface = SDL_LoadPNG("assets/xo-textures.png");
	m_pWindowIcon = SDL_LoadPNG("assets/icon.png");

	// Copy surface data to textures
	m_pBoardTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pBoardSurface);
	m_pXOTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pXOSurface);
	
	SDL_SetWindowIcon(m_pWindow, m_pWindowIcon);

	// Free up surface memory
	SDL_DestroySurface(m_pBoardSurface);
	SDL_DestroySurface(m_pXOSurface);

	// Load audio into memory and place onto SfxTrack
	m_pInvalidSelection = MIX_LoadAudio(m_pMixer, "assets/invalid-selection.wav", true);
	MIX_SetTrackAudio(m_pSfxTrack, m_pInvalidSelection);

	// Prevent scaled textures from blurring
	SDL_SetTextureScaleMode(m_pXOTexture, SDL_SCALEMODE_NEAREST);

	// Warm up audio device (reduces audio clipping)
	MIX_SetTrackGain(m_pSfxTrack, 0.0f);
	MIX_PlayTrack(m_pSfxTrack, 0);
	SDL_Delay(50);
	MIX_StopTrack(m_pSfxTrack, 0);
	MIX_SetTrackGain(m_pSfxTrack, 0.5f);

	SDL_SetRenderVSync(m_pRenderer, 1);

	Run();
}

/*    Private - TicTacToe    */

void Game::Run()
{
	// Main application loop
	while (m_windowRunning)
	{
		Input();
		Update();
		Render();
	}

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

void Game::Input()
{
	SDL_Event e{};
	while (SDL_PollEvent(&e))
	{
		// Begins application destruction on next frame
		if (e.type == SDL_EVENT_QUIT)
		{
			m_windowRunning = false;
		}

		// Handle ongoing game inputs
		switch (e.type)
		{
		case SDL_EVENT_MOUSE_BUTTON_DOWN: MouseEvent(e); break;
		default: break;
		}

		// Handle finished game inputs
		if (m_gameOver)
		{
			switch (e.type)
			{
			case SDL_EVENT_KEY_DOWN: KeyboardEvent(e); break;
			default: break;
			}
		}
	}
}

void Game::Update()
{
	// Time related tracking
	m_currentTime = SDL_GetTicks();

	// One tick per 50ms
	if (m_currentTime > m_lastTime + 50)
	{
		m_lastTime = m_currentTime;
		++m_ticks;
	}

	// Show the text for 40 ticks, or 2000ms
	if (m_invalidSelectionTextVisible)
	{
		if (m_ticks > m_invalidSelectionStartTick + 40)
		{
			m_invalidSelectionTextVisible = false;
		}
	}
}

void Game::Render()
{
	SDL_RenderClear(m_pRenderer);

	// Render board background & current state
	SDL_RenderTexture(m_pRenderer, m_pBoardTexture, nullptr, nullptr);
	RenderBoardState();

	RenderText();

	SDL_RenderPresent(m_pRenderer);
}

void Game::RenderBoardState()
{
	for (size_t index{}; index < m_boardStates.size(); ++index)
	{
		switch (m_boardStates.at(index))
		{
		case BoardState::VOID_STATE: break;
		case BoardState::X_STATE: RenderXTexture(index); break;
		case BoardState::O_STATE: RenderOTexture(index); break;
		default: break;
		}
	}
}

void Game::RenderText()
{
	SDL_SetRenderScale(m_pRenderer, 2, 2);
	
	// Render text depending on turn
	if (m_p1Turn && !m_gameOver)
	{
		SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
		SDL_RenderDebugText(m_pRenderer, 20, 20, "Turn: Player X");
	}
	else if (!m_p1Turn && !m_gameOver)
	{
		SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 255, 255);
		SDL_RenderDebugText(m_pRenderer, 20, 20, "Turn: Player O");
	}

	if (m_gameOver)
	{
		SDL_SetRenderDrawColor(m_pRenderer, 255, 160, 0, 255);
		SDL_RenderDebugText(m_pRenderer, 30, 20, "Game over!");

		switch (m_gameWinner)
		{
		case BoardState::X_STATE: 
			SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
			SDL_RenderDebugText(m_pRenderer, 10, 35, "Player X has won!");
			break;
		case BoardState::O_STATE:
			SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 255, 255);
			SDL_RenderDebugText(m_pRenderer, 10, 35, "Player O has won!");
			break;
		default:
			SDL_SetRenderDrawColor(m_pRenderer, 180, 0, 255, 255);
			SDL_RenderDebugText(m_pRenderer, 10, 35, "The game is tied!");
			break;
		}

		SDL_SetRenderDrawColor(m_pRenderer, 0, 255, 0, 255);
		SDL_RenderDebugText(m_pRenderer, 350 / 2, 685 / 2,
			"Would you like to play again? (y/n)");
	}

	if (m_invalidSelectionTextVisible)
	{
		SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
		SDL_RenderDebugText(m_pRenderer, 400 / 2, 685 / 2, "Invalid selection! Tile occupied.");
	}

	SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
	SDL_SetRenderScale(m_pRenderer, 1, 1);
}

void Game::RenderXTexture(const int index)
{
	// Each "space" within the sprite sheet is 16x16
	V2 texture = GetFromSpriteSheet(0, 0);
	const SDL_FRect src{ texture.x, texture.y, m_xoTextureSize, m_xoTextureSize };
	const SDL_FRect dst{ m_boardGrid.at(index) };

	// Texture tinted red
	SDL_SetTextureColorMod(m_pXOTexture, 255, 0, 0);

	SDL_RenderTexture(m_pRenderer, m_pXOTexture, &src, &dst);
}

void Game::RenderOTexture(const int index)
{
	V2 texture = GetFromSpriteSheet(1, 0);
	const SDL_FRect src{ texture.x, texture.y, m_xoTextureSize, m_xoTextureSize};
	const SDL_FRect dst{ m_boardGrid.at(index) };

	// Texture tinted blue
	SDL_SetTextureColorMod(m_pXOTexture, 0, 0, 255);

	SDL_RenderTexture(m_pRenderer, m_pXOTexture, &src, &dst);
}

void Game::MouseEvent(SDL_Event &e)
{
	SDL_MouseButtonEvent me{e.button};

	// Click requires button to be pressed & released
	if (me.clicks == 1)
	{
		const SDL_FPoint p{ me.x, me.y };

		// Disable placement if game is over
		if (!m_gameOver)
		{
			for (size_t index{}; index < m_boardGrid.size(); ++index)
			{
				if (SDL_PointInRectFloat(&p, &m_boardGrid.at(index)))
				{
					if (m_boardStates.at(index) != BoardState::VOID_STATE)
					{
						m_invalidSelectionTextVisible = true;
						m_invalidSelectionStartTick = m_ticks;
						PlayInvalidSfx();
					}
					else
					{
						PlaceOnGrid(index);
						
						// Win condition is only checked after each completed turn
						CheckWinConditions();
						m_p1Turn = !m_p1Turn;
					}
				}
			}
		}
	}
}

void Game::KeyboardEvent(SDL_Event &e)
{
	// Starts a new game if Y, closes window if N
	SDL_KeyboardEvent ke{ e.key };
	switch (ke.key)
	{
	case SDLK_Y:
		m_boardStates = m_initalBoardStates;
		m_gameWinner = BoardState::VOID_STATE;
		m_gameOver = false;
		break;
	case SDLK_N: 
		m_windowRunning = false;
		break;
	}
}

void Game::PlaceOnGrid(const int index)
{
	// Function called only on placement validation, so we don't check
	if (m_p1Turn)
	{
		m_boardStates.at(index) = BoardState::X_STATE;
	}
	else
	{
		m_boardStates.at(index) = BoardState::O_STATE;
	}
}

void Game::PlayInvalidSfx()
{
	// Track fades in to help prevent audio clipping
	MIX_StopTrack(m_pSfxTrack, 0);
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 5);
	MIX_PlayTrack(m_pSfxTrack, props);
	SDL_DestroyProperties(props);
}

V2 Game::GetFromSpriteSheet(const int x, const int y)
{
	V2 location{};

	if (x == 0 && y == 0)
	{
		return location;
	}
	else
	{
		// Sprite sheet contains one row, Y handling unnecessary
		location.x = x * m_xoTextureSize;

		return location;
	}
}

void Game::CheckWinConditions()
{
	BoardState state{};

	// Count the number of empty tiles
	int emptyTiles{};
	for (auto& i : m_boardStates)
	{
		if (i == BoardState::VOID_STATE)
		{
			++emptyTiles;
		}
	}

	// End game if all tiles are filled
	if (emptyTiles == 0)
	{
		state = BoardState::VOID_STATE;
		GameFinished(state);
	}

	// Set whose win condition to check depending on turn
	if (m_p1Turn)
	{
		state = BoardState::X_STATE;
	}
	else
	{
		state = BoardState::O_STATE;
	}

	auto ms2 = std::mdspan(m_boardStates.data(), 3, 3);

	// Check rows
	for (size_t rows{}; rows < ms2.extent(0); ++rows)
	{
		if (ms2[rows, 0] == state && ms2[rows, 1] == state && ms2[rows, 2] == state)
		{
			GameFinished(state);
		}
	}

	// Check columns
	for (size_t cols{}; cols < ms2.extent(1); ++cols)
	{
		if (ms2[0, cols] == state && ms2[1, cols] == state && ms2[2, cols] == state)
		{
			GameFinished(state);
		}
	}

	// Check diagonals
	if (ms2[0, 0] == state && ms2[1, 1] == state && ms2[2, 2] == state)
	{
		GameFinished(state);
	}
	else if (ms2[2, 0] == state && ms2[1, 1] == state && ms2[0, 2] == state)
	{
		GameFinished(state);
	}
}

void Game::GameFinished(BoardState& winner)
{
	m_gameOver = true;
	m_gameWinner = winner;
}