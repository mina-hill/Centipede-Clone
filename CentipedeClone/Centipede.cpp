#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;

// Initializing Dimensions.
// resolutionX and resolutionY determine the rendering resolution.
// Don't edit unless required. Use functions on lines 43, 44, 45 for resizing the game window.
const int resolutionX = 960;
const int resolutionY = 960;
const int boxPixelsX = 32;
const int boxPixelsY = 32;
const int gameRows = resolutionX / boxPixelsX; // Total rows on grid
const int gameColumns = resolutionY / boxPixelsY; // Total columns on grid

// Game states
enum GameState {
    MENU,
    PLAYING,
    INSTRUCTIONS,
    GAME_OVER,
    VICTORY
};

// Initializing GameGrid.
int gameGrid[gameRows][gameColumns] = {};

// The following exist purely for readability.
const int x = 0;
const int y = 1;
const int exists = 2;

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Write your functions declarations here. Some have been written for you. //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

void drawPlayer(sf::RenderWindow& window, float player[], sf::Sprite& playerSprite);
//draws the player if player exists
void movePlayer(float player[], sf::Sprite& playerSprite, const sf::Event& e);
//moves player 32 pixels using the arrow keys
void deadPlayer(float player[], float centipede[][4], int Csize);
//checks for collision between player and centipede

void moveBullet(float bullet[], sf::Clock& bulletClock);
//predefined function that moves bullet for 5ms
void drawBullet(sf::RenderWindow& window, float bullet[], sf::Sprite& bulletSprite);
//draws the bullet on the window as the bullet shoots upwards
void shootBullet(float player[], float bullet[]);
//checks if the player has pressed space bar and shoots if player has

void drawCentipede(sf::RenderWindow& window, float centipede[][4], sf::Sprite& centipedeHeadSprite, sf::Sprite& centipedeBodySprite, bool segmentIsHead[], int Csize);
//draws centipede using the array of centipede if centipede exists
void moveCentipedeLR(float centipede[][4], sf::Clock& centipedeClock, int &count, int size, bool &cDown, bool &mDown, bool cUp, bool &noOtherMushroom, bool segmentIsHead[], int Csize);
//moves centipede,moves centipededown if it collides with the walls or mushrooms
int splitC(float centipede[][4], float bullet[], int &bodyC, int &headC, bool &split, bool segmentIsHead[], bool &RightD, int &numMushrooms, int Csize);
//splits the centipede if bullet hits the centipede
void mVSc(float centipede[][4], bool &cDown, bool &mDown, bool &noOtherMushroom, int Csize);
//checks forcollision between centipede and mushrooms, allowing centipede to move down if it collides with mushroom

void mushrooms(float mushroom[], int &numMushrooms); 
//initializes the positions of the mushrooms on the gameGrid
void drawMushrooms(sf::RenderWindow& window, sf::Sprite mushroomSprite, sf::Sprite halfMushroomSprite, sf::Sprite poisonSprite, sf::Sprite halfPoisonSprite);
//uses the gameGrid to draw the functions in their corresponding positions in the window
void destroyMushroom(float mushroom[], float bullet[], int &mushroomDead);
//destroys mushrooms if bullet collides with it

void drawGrid();
//draws grid, helps to analyze in the formulation of the code
void game(float centipede[][4], int mushroomDead,int bodyC, int headC, bool &gameOver, float player[],int Csize);
//checksif game is over,player is dead or all centipede segments are dead

// Menu functions
void drawMenu(sf::RenderWindow& window, sf::Font& font, int selectedOption);
void drawInstructions(sf::RenderWindow& window, sf::Font& font);
void resetGame(float player[], float bullet[], float centipede[][4], bool segmentIsHead[], int Csize, int& score, int& mushroomDead, int& bodyC, int& headC, bool& gameOver);

int main()
{
	srand(time(0));

	// Declaring RenderWindow.
	sf::RenderWindow window(sf::VideoMode(resolutionX, resolutionY), "Centipede", sf::Style::Close | sf::Style::Titlebar);
	
	// Used to resize your window if it's too big or too small. Use according to your needs.
	window.setSize(sf::Vector2u(640, 640)); // Recommended for 1366x768 (768p) displays.
	// window.setSize(sf::Vector2u(1280, 1280)); // Recommended for 2560x1440 (1440p) displays.
	// window.setSize(sf::Vector2u(1920, 1920)); // Recommended for 3840x2160 (4k) displays.
	
	// Used to position your window on every launch. Use according to your needs.
	window.setPosition(sf::Vector2i(100, 0));

	// Initialize game state
	GameState gameState = MENU;
	int selectedMenuOption = 0; // 0 = Play, 1 = Instructions, 2 = Quit
	
	// Initializing Background Music.
	sf::Music bgMusic;
	bgMusic.openFromFile("Music/field_of_hopes.ogg");
	bgMusic.setLoop(true);
	bgMusic.play();
	bgMusic.setVolume(50);

	// Initializing Background.
	sf::Texture backgroundTexture;
	sf::Sprite backgroundSprite;
	backgroundTexture.loadFromFile("Textures/jkk.jpg");
	backgroundSprite.setTexture(backgroundTexture);
	backgroundSprite.setColor(sf::Color(255, 255, 255, 255 * 0.50)); // Reduces Opacity to 30%
	
	// Initializing Player and Player Sprites.
	float player[3] = {}; // Changed from [2] to [3] to include the 'exists' element
	player[x] = (gameColumns / 2) * boxPixelsX;
	player[y] = 26* boxPixelsY;//(gameColumns * 3 / 4) * boxPixelsY;
	player[exists] = true;
	sf::Texture playerTexture;
	sf::Sprite playerSprite;
	playerTexture.loadFromFile("Textures/player.png");
	playerSprite.setTexture(playerTexture);
	playerSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
	
	// Initializing Bullet and Bullet Sprites.
	float bullet[3] = {};
	bullet[x] = player[x];
	bullet[y] = player[y] - boxPixelsY;
	bullet[exists] = false;
	sf::Clock bulletClock;
	sf::Texture bulletTexture;
	sf::Sprite bulletSprite;
	bulletTexture.loadFromFile("Textures/bullet.png");
	bulletSprite.setTexture(bulletTexture);
	bulletSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
	
	//Initializing Centipede 
	const int Csize = 12;
	float centipede[Csize][4];
	centipede[0][x] = 544;//(rand() % gameColumns+1) * boxPixelsX;//544;(
	centipede[0][y] = (rand() % (gameRows-5) + 1) * boxPixelsY;//0;
	
	int size=Csize;
	int splitIndex;
	bool split= false;
	bool segmentIsHead[Csize];
	for(int i=0;i<Csize;i++)
	{
		centipede[i][exists] = true;
		centipede[i][3] = false;//when its false it moves left
		segmentIsHead[i]=false;
		segmentIsHead[0]=true;
	}
	
	bool cDown= false;
	bool cUp = false;
	bool noOtherMushroom= false;
	static int count =0;
	bool RightD = count%2;
	sf::Clock centipedeClock;
	
	//Centipede Head Sprites.
	sf::Texture centipedeHeadTexture;
	sf::Sprite centipedeHeadSprite;
	centipedeHeadTexture.loadFromFile("Textures/c_head_left.png");
	centipedeHeadSprite.setTexture(centipedeHeadTexture);
	centipedeHeadSprite.setRotation(0.f);
	centipedeHeadSprite.setOrigin(centipedeHeadSprite.getLocalBounds().width / 2, centipedeHeadSprite.getLocalBounds().height / 2);
	centipedeHeadSprite.setTextureRect(sf::IntRect(0, 0, 28, boxPixelsY));
	//Centipede Body Sprites.
	sf::Texture centipedeBodyTexture;
	sf::Sprite centipedeBodySprite;
	centipedeBodyTexture.loadFromFile("Textures/c_body_left.png");
	centipedeBodySprite.setTexture(centipedeBodyTexture);
	centipedeBodySprite.setTextureRect(sf::IntRect(0, 0, 28, boxPixelsY));
	bool mDown=false;
	for (int i = 1; i < Csize; i++) 
		{
			if(!centipede[i][3])
			{
				centipede[i][x] = centipede[i - 1][x] + 32;
				centipede[i][y] = centipede[i - 1][y];
			}
			else if(centipede[i][3])
			{
				centipede[i][x] = centipede[i - 1][x] - 32;
				centipede[i][y] = centipede[i - 1][y];
			}
		}
	
	//Mushroom sprites.
	float mushroom[3] = {};
	float poison[3] = {};
	int numMushrooms=0;
	//Entire Mushroom
	mushroom[exists]=true;
	sf::Texture mushroomTexture;
	sf::Sprite mushroomSprite;
	mushroomTexture.loadFromFile("Textures/mushroom.png");
	mushroomSprite.setTexture(mushroomTexture);
	mushroomSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
	//Half mushroom
	sf::Texture halfMushroomTexture;
	sf::Sprite halfMushroomSprite;
	halfMushroomTexture.loadFromFile("Textures/halfMushroom.png");
	halfMushroomSprite.setTexture(halfMushroomTexture);
	halfMushroomSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
	//poison mushroom
	poison[exists]=true;
	sf::Texture poisonTexture;
	sf::Sprite poisonSprite;
	poisonTexture.loadFromFile("Textures/mushroom.png");
	poisonSprite.setTexture(poisonTexture);
	poisonSprite.setTextureRect(sf::IntRect(0, 32, boxPixelsX, boxPixelsY));
	//Half Poison mushroom
	sf::Texture halfPoisonTexture;
	sf::Sprite halfPoisonSprite;
	halfPoisonTexture.loadFromFile("Textures/mushroom.png");
	halfPoisonSprite.setTexture(halfPoisonTexture);
	halfPoisonSprite.setTextureRect(sf::IntRect(64, 32, boxPixelsX, boxPixelsY));
	
	//Scoring
	bool gameOver=false;
	int score = 0, mushroomDead = 0, bodyC =0, headC=0;
	
	// Load game over image
	sf::Texture gameOverTexture;
	sf::Sprite gameOverSprite;
	gameOverTexture.loadFromFile("Textures/game_over.png");
	gameOverSprite.setTexture(gameOverTexture);
	// Center the game over image
	gameOverSprite.setOrigin(gameOverSprite.getLocalBounds().width / 2, gameOverSprite.getLocalBounds().height / 2);
	gameOverSprite.setPosition(resolutionX / 2, resolutionY / 2);
	
	// Load victory image
	sf::Texture victoryTexture;
	sf::Sprite victorySprite;
	victoryTexture.loadFromFile("Textures/you_won.png");
	victorySprite.setTexture(victoryTexture);
	// Center the victory image
	victorySprite.setOrigin(victorySprite.getLocalBounds().width / 2, victorySprite.getLocalBounds().height / 2);
	victorySprite.setPosition(resolutionX / 2, resolutionY / 2);
	
	sf::Font font;
	font.loadFromFile("arial.ttf");
	sf::Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(30);
	scoreText.setFillColor(sf::Color::White);
	// Set score text position consistently
	scoreText.setPosition(20, 20); // Top-left corner with a small margin
	
	while(window.isOpen()) {
		sf::Event e;
		while (window.pollEvent(e)) {
			if (e.type == sf::Event::Closed) {
				return 0;
			}
			
			// Handle menu input
			if (gameState == MENU) {
				if (e.type == sf::Event::KeyPressed) {
					if (e.key.code == sf::Keyboard::Up) {
						selectedMenuOption = (selectedMenuOption - 1 + 3) % 3; // Cycle through options (0,1,2)
					}
					else if (e.key.code == sf::Keyboard::Down) {
						selectedMenuOption = (selectedMenuOption + 1) % 3; // Cycle through options (0,1,2)
					}
					else if (e.key.code == sf::Keyboard::Return) {
						switch (selectedMenuOption) {
							case 0: // Play Game
								gameState = PLAYING;
								// Reset game state if coming from game over
								if (gameOver) {
									resetGame(player, bullet, centipede, segmentIsHead, Csize, score, mushroomDead, bodyC, headC, gameOver);
									bgMusic.play();
								}
								// Always initialize new mushrooms when starting a new game
								mushroom[exists] = true;
								mushrooms(mushroom, numMushrooms);
								break;
							case 1: // Instructions
								gameState = INSTRUCTIONS;
								break;
							case 2: // Quit
								return 0;
								break;
						}
					}
				}
			}
			// Return to menu from instructions
			else if (gameState == INSTRUCTIONS) {
				if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
					gameState = MENU;
				}
			}
			// Return to menu from game over
			else if (gameState == GAME_OVER) {
				if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
					gameState = MENU;
				}
			}
			// Return to menu from victory screen
			else if (gameState == VICTORY) {
				if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
					gameState = MENU;
				}
			}
			// Handle game input
			else if (gameState == PLAYING) {
				if (e.type == sf::Event::MouseButtonPressed) {
					shootBullet(player, bullet);
				}
				else if (e.type == sf::Event::KeyPressed) {
					if (e.key.code == sf::Keyboard::Space)
						shootBullet(player, bullet);
					else if (e.key.code == sf::Keyboard::Escape)
						gameState = MENU;
					else {
						movePlayer(player, playerSprite, e);
					}
				}
			}
		}
		
		window.clear();
		window.draw(backgroundSprite);
		
		// Handle different game states
		if (gameState == MENU) {
			drawMenu(window, font, selectedMenuOption);
		}
		else if (gameState == INSTRUCTIONS) {
			drawInstructions(window, font);
		}
		else if (gameState == VICTORY) {
			// Draw victory screen
			window.draw(victorySprite);
			
			// Continue to show the regular score in its usual position
			scoreText.setString("Score: " + std::to_string(score));
			window.draw(scoreText);
			
			// Add a separate final score text in the center
			sf::Text finalScoreText;
			finalScoreText.setFont(font);
			finalScoreText.setCharacterSize(36);
			finalScoreText.setFillColor(sf::Color::Yellow);
			finalScoreText.setString("Final Score: " + std::to_string(score));
			finalScoreText.setOrigin(finalScoreText.getLocalBounds().width / 2, 0);
			finalScoreText.setPosition(resolutionX / 2, resolutionY / 2 + 80);
			window.draw(finalScoreText);
			
			// Add "Press ESC to return to menu" text
			sf::Text returnText;
			returnText.setFont(font);
			returnText.setCharacterSize(24);
			returnText.setFillColor(sf::Color::White);
			returnText.setString("Press ESC to return to menu");
			returnText.setOrigin(returnText.getLocalBounds().width / 2, 0);
			returnText.setPosition(resolutionX / 2, resolutionY / 2 + 150);
			window.draw(returnText);
		}
		else if (gameState == PLAYING || gameState == GAME_OVER) {
			// Game logic
			game(centipede, mushroomDead, bodyC, headC, gameOver, player, Csize);
			
			// Check if player just died - stop the music and change state
			static bool musicStopped = false;
			if (gameOver && !player[exists]) {
				if (!musicStopped) {
					bgMusic.stop();
					musicStopped = true;
				}
				gameState = GAME_OVER;
			}
			
			// Check if all centipede segments are destroyed - trigger victory
			bool allCentipedeDestroyed = true;
			for(int i=0; i<Csize; i++) {
				if(centipede[i][exists]) {
					allCentipedeDestroyed = false;
					break;
				}
			}
			
			if(allCentipedeDestroyed && gameState == PLAYING) {
				gameState = VICTORY;
				// Change music or add victory sound effect here if desired
			}
			
			score = (1*mushroomDead) + (10*bodyC) + (20*headC);
			
			if (player[exists])
				drawPlayer(window, player, playerSprite);
			
			if (bullet[exists] == true) {
				moveBullet(bullet, bulletClock);
				drawBullet(window, bullet, bulletSprite);
			}
			
			if (bullet[exists]) {
				moveBullet(bullet, bulletClock);
				drawBullet(window, bullet, bulletSprite);
				destroyMushroom(mushroom, bullet, mushroomDead);
				splitC(centipede, bullet, bodyC, headC, split, segmentIsHead, RightD, numMushrooms, Csize);
			}
			
			// Only update centipede movement if the player is still alive and game is in playing state
			if (gameState == PLAYING && player[exists]) {
				cDown = false;
				moveCentipedeLR(centipede, centipedeClock, count, size, cDown, mDown, cUp, noOtherMushroom, segmentIsHead, Csize);
				deadPlayer(player, centipede, Csize);
			}
			
			// Draw the centipede
			drawCentipede(window, centipede, centipedeHeadSprite, centipedeBodySprite, segmentIsHead, Csize);
			
			// Remove the automatic mushroom creation from the game loop
			// to prevent new mushrooms from appearing during gameplay
			drawMushrooms(window, mushroomSprite, halfMushroomSprite, poisonSprite, halfPoisonSprite);
			
			// Update score text but don't change its position
			scoreText.setString("Score: " + std::to_string(score));
			window.draw(scoreText);
			
			// Display game over message if player is dead
			if (gameState == GAME_OVER) {
				window.draw(gameOverSprite);
				
				// Add "Press ESC to return to menu" text
				sf::Text returnText;
				returnText.setFont(font);
				returnText.setCharacterSize(24);
				returnText.setFillColor(sf::Color::White);
				returnText.setString("Press ESC to return to menu");
				returnText.setOrigin(returnText.getLocalBounds().width / 2, 0);
				returnText.setPosition(resolutionX / 2, resolutionY / 2 + 100);
				window.draw(returnText);
			}
		}
		
		window.display();
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Write your functions definitions here. Some have been written for you. //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

void drawPlayer(sf::RenderWindow& window, float player[], sf::Sprite& playerSprite) {
	playerSprite.setPosition(player[x], player[y]);
	if(player[exists])
	window.draw(playerSprite);
}
void movePlayer(float player[], sf::Sprite& playerSprite, const sf::Event& e)
{
	int moveX=0;
	int moveY=0;

	if(e.key.code == sf::Keyboard::Up && player[y]>(resolutionY-(boxPixelsY*5)))
		moveY--;
	else if(e.key.code == sf::Keyboard::Down && player[y]<(resolutionY-boxPixelsY))
		moveY++;
	else if(e.key.code == sf::Keyboard::Right && player[x]<(resolutionY-boxPixelsX))
		moveX++;
	else if(e.key.code == sf::Keyboard::Left && player[x]>0)
		moveX--;
		
	player[x]+=moveX*boxPixelsX;
	player[y]+=moveY*boxPixelsY;
}
void deadPlayer(float player[], float centipede[][4], int Csize)
{
	for(int i=0; i<Csize; i++)
	{
		if(centipede[i][exists])
		{
			// Check for actual overlap between player and centipede using proper collision detection
			// Only detect collision if there's a significant overlap (at least 16 pixels - half the sprite size)
			bool xOverlap = (player[x] < centipede[i][x] + boxPixelsX - 8) && (player[x] + boxPixelsX - 8 > centipede[i][x]);
			bool yOverlap = (player[y] < centipede[i][y] + boxPixelsY - 8) && (player[y] + boxPixelsY - 8 > centipede[i][y]);
			
			if(xOverlap && yOverlap)
			{
				player[exists] = false;
				break;
			}
		}
	}
}

void moveBullet(float bullet[], sf::Clock& bulletClock) {
	if (bulletClock.getElapsedTime().asMilliseconds() < 5)
		return;

	bulletClock.restart();
	bullet[y] -= 10;	
	if (bullet[y] < -32)
		bullet[exists] = false;
}
void drawBullet(sf::RenderWindow& window, float bullet[], sf::Sprite& bulletSprite) {
	bulletSprite.setPosition(bullet[x], bullet[y]);
	window.draw(bulletSprite);
}
void shootBullet(float player[], float bullet[])
{	///shoots according to player position
	if(player[exists])
	{
		bullet[x] = player[x];
		bullet[y] = player[y] - boxPixelsY;
		bullet[exists] = true;
	}
}

void drawCentipede(sf::RenderWindow& window, float centipede[][4], sf::Sprite& centipedeHeadSprite, sf::Sprite& centipedeBodySprite, bool segmentIsHead[], int Csize)
{
	centipedeHeadSprite.setOrigin(0,0);
	centipedeHeadSprite.setPosition(centipede[0][x], centipede[0][y]);
	centipedeBodySprite.setOrigin(0,0);
	
	for(int i=0;i<Csize; i++)
	{  ///draws the centipede according to the array while checking if it is is body or head
		if(centipede[i][exists]==true)
		{	
			if(segmentIsHead[i])
			{	
				centipedeHeadSprite.setPosition(centipede[i][x], centipede[i][y]);
				window.draw(centipedeHeadSprite);
			}
			else
			{
				centipedeBodySprite.setPosition(centipede[i][x], centipede[i][y]);
				window.draw(centipedeBodySprite);
			}
		}
	}
	
}
void moveCentipedeLR(float centipede[][4], sf::Clock& centipedeClock, int &count,int size, bool &cDown, bool &mDown, bool cUp, bool &noOtherMushroom, bool segmentIsHead[], int Csize)
{	
	if (centipedeClock.getElapsedTime().asMilliseconds() <= 200)
		return;
	centipedeClock.restart();
	
	// Reset flags at the beginning of movement update
	cDown = false;
	mDown = false;
	
	// Create a dynamic array for previous positions
	float** prevPositions = new float*[Csize];
	for(int i = 0; i < Csize; i++) {
		prevPositions[i] = new float[2];
		if(centipede[i][exists]) {
			prevPositions[i][0] = centipede[i][x];
			prevPositions[i][1] = centipede[i][y];
		}
	}
	
	// First handle the head movement (whether original head or any new heads from splitting)
	for(int i=0; i<Csize; i++) {
		if(!centipede[i][exists] || !segmentIsHead[i])
			continue;
			
		// First check if we need to change direction due to wall or mushroom collision
		bool needsDirectionChange = false;
		
		// Check wall collisions for the head
		if(centipede[i][x] <= 0 && !centipede[i][3]) // Left wall collision
		{
			needsDirectionChange = true;
			cDown = true;
		}
		else if(centipede[i][x] >= (resolutionX - boxPixelsX) && centipede[i][3]) // Right wall collision
		{
			needsDirectionChange = true;
			cDown = true;
		}
		
		// Check mushroom collisions for the head only
		int headGridX = centipede[i][x] / boxPixelsX;
		int headGridY = centipede[i][y] / boxPixelsY;
		int nextCellX;
		
		if(centipede[i][3]) { // Moving right
			nextCellX = headGridX + 1;
			if(nextCellX < gameRows && (gameGrid[nextCellX][headGridY] == 1 || gameGrid[nextCellX][headGridY] == 3)) {
				needsDirectionChange = true;
				mDown = true;
			}
		} else { // Moving left
			nextCellX = headGridX - 1;
			if(nextCellX >= 0 && (gameGrid[nextCellX][headGridY] == 1 || gameGrid[nextCellX][headGridY] == 3)) {
				needsDirectionChange = true;
				mDown = true;
			}
		}
		
		// Move the head based on conditions
		if(needsDirectionChange) {
			centipede[i][y] += boxPixelsY; // Move down one row
			centipede[i][3] = !centipede[i][3]; // Reverse direction
		} else {
			// Otherwise just move horizontally
			if(!centipede[i][3]) // Move left
				centipede[i][x] -= boxPixelsX;
			else // Move right
				centipede[i][x] += boxPixelsX;
		}
	}
	
	// Now move all body segments to follow the segment in front of them
	for(int i=0; i<Csize; i++) {
		if(!centipede[i][exists] || segmentIsHead[i])
			continue;
			
		// Find the segment this one follows
		int followIndex = -1;
		for(int j=0; j<Csize; j++) {
			if(j != i && centipede[j][exists]) {
				// If segments are adjacent in the array, they're likely connected
				if(abs(j-i) == 1) {
					// The lower index is usually in front
					if(j < i) {
						followIndex = j;
						break;
					}
				}
			}
		}
		
		// If we found a segment to follow, move to its previous position
		if(followIndex >= 0) {
			centipede[i][x] = prevPositions[followIndex][0];
			centipede[i][y] = prevPositions[followIndex][1];
			// Inherit direction from the segment we're following
			centipede[i][3] = centipede[followIndex][3];
		}
	}
	
	// Free the dynamically allocated memory
	for(int i = 0; i < Csize; i++) {
		delete[] prevPositions[i];
	}
	delete[] prevPositions;
	
	// Check if any segment is at the bottom of player area
	for(int i=0; i<Csize; i++) {
		if(centipede[i][exists] && centipede[i][y] <= resolutionY - boxPixelsY) {
			cUp = true;
			break;
		}
	}
	
	noOtherMushroom = false;
	count++;
}
int splitC(float centipede[][4], float bullet[], int &bodyC, int &headC, bool &split, bool segmentIsHead[], bool &RightD, int &numMushrooms, int Csize)
{
	int index = -1;
	int splitx, splity;
	
	// Check for collision between bullet and centipede segments
	for(int i=0; i<Csize; i++)
	{
		if(!centipede[i][exists])
			continue;
			
		// Check if bullet hit this segment
		if(bullet[y] >= centipede[i][y] && bullet[y] <= centipede[i][y] + boxPixelsY && 
		   centipede[i][x] <= bullet[x] && centipede[i][x] + boxPixelsX >= bullet[x])
		{	
			// Store the hit position to place a mushroom
			splitx = (bullet[x] / boxPixelsX);
			splity = (bullet[y] / boxPixelsY);
			
			// Find the original head's direction before we destroy this segment
			bool originalHeadDirection = false; // Default direction (left)
			for(int j=0; j<Csize; j++) {
				if(centipede[j][exists] && segmentIsHead[j]) {
					originalHeadDirection = centipede[j][3];
					break;
				}
			}
			
			// Mark the segment as destroyed
			centipede[i][exists] = false;
			bullet[exists] = false;
			
			// Add score based on segment type
			if(segmentIsHead[i]) {
				headC++;
				
				// If the head was hit, then the next segment becomes a new head with same direction
				if(i < Csize - 1 && centipede[i+1][exists]) {
					segmentIsHead[i+1] = true;
					// Keep the same direction as before
				}
			} else {
				bodyC++;
				
				// If a body segment was hit, create two separate centipedes
				
				// Find the break point - divide into two groups
				bool foundNewHead = false;
				
				// Make the next segment after the hit a new head, moving in opposite direction
				if(i < Csize - 1 && centipede[i+1][exists]) {
					segmentIsHead[i+1] = true;
					// Set direction opposite to the original head
					centipede[i+1][3] = !originalHeadDirection;
					foundNewHead = true;
				}
				
				// Also check for segments before the hit point to make sure we're not missing any
				if(!foundNewHead && i > 0 && centipede[i-1][exists]) {
					segmentIsHead[i-1] = true;
					// Set direction opposite to the original head
					centipede[i-1][3] = !originalHeadDirection;
				}
			}
			
			// Track that we split the centipede
			split = true;
			index = i;
			
			// Add a mushroom at the hit location
			numMushrooms++;
			if(splity >= 25)
				gameGrid[splitx][splity] = 2; // Poison mushroom
			else
				gameGrid[splitx][splity] = 1; // Regular mushroom
			
			// No need to check other segments once we found a hit
			break;
		}
	}
	
	return index;
}
void mVSc(float centipede[][4], bool &cDown, bool &mDown, bool &noOtherMushroom, int Csize)
{
	for(int k=0; k<Csize; k++)
	{
		if(!centipede[k][exists])
			continue;

		// Get centipede segment position in grid coordinates
		int centipedeGridX = centipede[k][x] / boxPixelsX;
		int centipedeGridY = centipede[k][y] / boxPixelsY;
		
		// Check the next cell in the direction the centipede is moving
		int nextCellX;
		if(centipede[k][3]) // Moving right
		{
			nextCellX = centipedeGridX + 1;
			// If we're about to exceed the grid bounds, skip this check
			if(nextCellX >= gameRows)
				continue;
				
			// If there's a mushroom in the next cell, signal to change direction
			if(gameGrid[nextCellX][centipedeGridY] == 1 || gameGrid[nextCellX][centipedeGridY] == 3)
			{
				mDown = true;
				return;
			}
		}
		else // Moving left
		{
			nextCellX = centipedeGridX - 1;
			// If we're about to exceed the grid bounds, skip this check
			if(nextCellX < 0)
				continue;
				
			// If there's a mushroom in the next cell, signal to change direction
			if(gameGrid[nextCellX][centipedeGridY] == 1 || gameGrid[nextCellX][centipedeGridY] == 3)
			{
				mDown = true;
				return;
			}
		}
		
		// Also check if there are any mushrooms below
		if(centipedeGridY + 1 < gameColumns)
		{
			if(gameGrid[centipedeGridX][centipedeGridY + 1] == 0)
			{
				noOtherMushroom = true;
			}
		}
	}
}

void drawMushrooms(sf::RenderWindow& window, sf::Sprite mushroomSprite, sf::Sprite halfMushroomSprite, sf::Sprite poisonSprite, sf::Sprite halfPoisonSprite)
{
	for(int i=0;i<gameRows; i++)
	{
		for(int j=0;j<gameColumns; j++)
		{
			if(gameGrid[i][j]==1)
			{	//full mushroom
				mushroomSprite.setPosition(i * boxPixelsX, j * boxPixelsY);
				window.draw(mushroomSprite);
			}
			else if(gameGrid[i][j]==3)
			{	//half mushroom
				halfMushroomSprite.setPosition(i * boxPixelsX, j * boxPixelsY);
				window.draw(halfMushroomSprite);
			}
			else if(gameGrid[i][j]==2)
			{	//full poisonous mushroom
				poisonSprite.setPosition(i * boxPixelsX, j * boxPixelsY);
				window.draw(poisonSprite);
			}
			else if(gameGrid[i][j]==4)
			{	//half poisonous mushroom
				halfPoisonSprite.setPosition(i * boxPixelsX, j * boxPixelsY);
				window.draw(halfPoisonSprite);
			}
		}
	}
}
void mushrooms(float mushroom[],int &numMushrooms)
{	//initializing the positions of mushrooms on the grid
	numMushrooms = rand() % 11 + 20;
	for(int i=0; i<numMushrooms; i++)
	{
		int xM = (rand() % (gameRows)) ;
		int yM = (rand() % (gameColumns-5));
	
		gameGrid[xM][yM] = 1; 
	}
	mushroom[exists] = false;
	
}
void destroyMushroom(float mushroom[], float bullet[], int &mushroomDead)
{
	for(int i=0;i<gameRows; i++)
	{
		for(int j=0;j<gameColumns; j++)
		{	
			if((gameGrid[j][i]==1 || gameGrid[j][i]==2) && (i*boxPixelsY+32 > bullet[y]) && (j*boxPixelsX < bullet[x]+16 && j*boxPixelsX + 32 > bullet[x]+16))
			{	//bullet first hit, half mushroom
				gameGrid[j][i]=3;
				bullet[exists] = false;
				return;
				
			}
			if((gameGrid[j][i]==3 || gameGrid[j][i]==4) && (i*boxPixelsY+32 > bullet[y]) && (j*boxPixelsX < bullet[x]+16 && j*boxPixelsX + 32 > bullet[x]+16))
			{	//bullet 2nd hit,mushrooms gone
				gameGrid[j][i]=0;
				bullet[exists] = false;
				mushroomDead++;
				return;
			}
		}
	}
}

void drawGrid()
{
	for(int i=0;i<gameRows; i++)
	{
		for(int j=0;j<gameColumns; j++)
		{
			cout<<gameGrid[j][i]<<' ';
		}
		cout<<endl;
	}
	cout<<endl<<endl;
}

void game(float centipede[][4], int mushroomDead,int bodyC, int headC, bool &gameOver, float player[],int Csize)
{
	// Check if player is dead
	if(player[exists] == 0) {
		gameOver = true;
		return;
	}
	
	// Check if all centipede segments are destroyed
	bool allDestroyed = true;
	for(int i=0; i<Csize; i++) {
		if(centipede[i][exists]) {
			allDestroyed = false;
			break;
		}
	}
	
	// Only set gameOver to true if player is dead
	// If all centipede segments are destroyed, the victory state will be triggered separately
	if(allDestroyed) {
		gameOver = true;
	}
}

// Menu functions
void drawMenu(sf::RenderWindow& window, sf::Font& font, int selectedOption) {
	// Title
	sf::Text titleText;
	titleText.setFont(font);
	titleText.setCharacterSize(60);
	titleText.setFillColor(sf::Color::Yellow);
	titleText.setString("CENTIPEDE");
	titleText.setOrigin(titleText.getLocalBounds().width / 2, 0);
	titleText.setPosition(resolutionX / 2, 100);
	window.draw(titleText);
	
	// Menu options
	sf::Text playText, instructionsText, quitText;
	playText.setFont(font);
	instructionsText.setFont(font);
	quitText.setFont(font);
	
	playText.setCharacterSize(36);
	instructionsText.setCharacterSize(36);
	quitText.setCharacterSize(36);
	
	// Highlight selected option
	playText.setFillColor(selectedOption == 0 ? sf::Color::Green : sf::Color::White);
	instructionsText.setFillColor(selectedOption == 1 ? sf::Color::Green : sf::Color::White);
	quitText.setFillColor(selectedOption == 2 ? sf::Color::Green : sf::Color::White);
	
	playText.setString("Play Game");
	instructionsText.setString("Instructions");
	quitText.setString("Quit");
	
	playText.setOrigin(playText.getLocalBounds().width / 2, 0);
	instructionsText.setOrigin(instructionsText.getLocalBounds().width / 2, 0);
	quitText.setOrigin(quitText.getLocalBounds().width / 2, 0);
	
	playText.setPosition(resolutionX / 2, 300);
	instructionsText.setPosition(resolutionX / 2, 380);
	quitText.setPosition(resolutionX / 2, 460);
	
	window.draw(playText);
	window.draw(instructionsText);
	window.draw(quitText);
	
	// Navigation hint
	sf::Text navigationText;
	navigationText.setFont(font);
	navigationText.setCharacterSize(24);
	navigationText.setFillColor(sf::Color::White);
	navigationText.setString("Use UP/DOWN arrows to navigate, ENTER to select");
	navigationText.setOrigin(navigationText.getLocalBounds().width / 2, 0);
	navigationText.setPosition(resolutionX / 2, resolutionY - 100);
	window.draw(navigationText);
}

void drawInstructions(sf::RenderWindow& window, sf::Font& font) {
	sf::Text titleText;
	titleText.setFont(font);
	titleText.setCharacterSize(48);
	titleText.setFillColor(sf::Color::Yellow);
	titleText.setString("INSTRUCTIONS");
	titleText.setOrigin(titleText.getLocalBounds().width / 2, 0);
	titleText.setPosition(resolutionX / 2, 50);
	window.draw(titleText);
	
	sf::Text instructionsText;
	instructionsText.setFont(font);
	instructionsText.setCharacterSize(24);
	instructionsText.setFillColor(sf::Color::White);
	
	std::string instructions = 
		"OBJECTIVE:\n"
		"Shoot the centipede to score points!\n\n"
		"CONTROLS:\n"
		"- Arrow Keys: Move player\n"
		"- Space/Mouse Click: Shoot\n"
		"- ESC: Return to menu\n\n"
		"SCORING:\n"
		"- Mushroom: 1 point\n"
		"- Centipede body segment: 10 points\n"
		"- Centipede head: 20 points\n\n"
		"TIPS:\n"
		"- When you hit a centipede segment, it splits and the new head\n"
		"  travels in the opposite direction\n"
		"- Avoid touching any centipede segment or you'll lose!";
	
	instructionsText.setString(instructions);
	instructionsText.setPosition(resolutionX / 2 - 350, 150);
	window.draw(instructionsText);
	
	sf::Text returnText;
	returnText.setFont(font);
	returnText.setCharacterSize(24);
	returnText.setFillColor(sf::Color::Yellow);
	returnText.setString("Press ESC to return to menu");
	returnText.setOrigin(returnText.getLocalBounds().width / 2, 0);
	returnText.setPosition(resolutionX / 2, resolutionY - 100);
	window.draw(returnText);
}

void resetGame(float player[], float bullet[], float centipede[][4], bool segmentIsHead[], int Csize, int& score, int& mushroomDead, int& bodyC, int& headC, bool& gameOver) {
	// Reset player
	player[x] = (gameColumns / 2) * boxPixelsX;
	player[y] = 26 * boxPixelsY;
	player[exists] = true;
	
	// Reset bullet
	bullet[exists] = false;
	
	// Reset centipede
	centipede[0][x] = 544;
	centipede[0][y] = (rand() % (gameRows-5) + 1) * boxPixelsY;
	
	for(int i=0; i<Csize; i++) {
		centipede[i][exists] = true;
		centipede[i][3] = false; // Moving left
		segmentIsHead[i] = false;
	}
	segmentIsHead[0] = true;
	
	// Reposition body segments
	for (int i = 1; i < Csize; i++) {
		if(!centipede[i][3]) {
			centipede[i][x] = centipede[i - 1][x] + 32;
			centipede[i][y] = centipede[i - 1][y];
		}
		else {
			centipede[i][x] = centipede[i - 1][x] - 32;
			centipede[i][y] = centipede[i - 1][y];
		}
	}
	
	// Reset game grid (clear mushrooms)
	for(int i=0; i<gameRows; i++) {
		for(int j=0; j<gameColumns; j++) {
			gameGrid[i][j] = 0;
		}
	}
	
	// Reset score and game state
	score = 0;
	mushroomDead = 0;
	bodyC = 0;
	headC = 0;
	gameOver = false;
	
	// Reset music stopped flag
	static bool musicStopped = false;
	musicStopped = false;
}
