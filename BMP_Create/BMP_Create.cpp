#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>

// Level properties
const int Level = 0;
int currentLevel = Level;

const int brickColumns = 10;

std::vector<sf::RectangleShape> bricks; // Container of bricks

// Define Debris before using it in a vector
struct Debris {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
    float rotationSpeed;
};

std::vector<Debris> debris;

const int brickRows = 3;
int currentbrickRows = brickRows;

const int maxBalls = 3;
int remainingBalls = maxBalls;

const int brickHitDebris = 10;

sf::Clock levelClock;
sf::Clock debrisClock;

// Initialize variables for staggered brick falling
bool lastRowFalling = false;
size_t currentBrickIndex = bricks.size() - brickColumns; // Start from the first brick in the last row
sf::Clock brickFallClock; // Separate clock for brick falling timing

// Helper function for ball collision remains unchanged
void resolveCollision(sf::Vector2f& pos1, sf::Vector2f& vel1, sf::Vector2f& pos2, sf::Vector2f& vel2, float radius) {
    sf::Vector2f diff = pos1 - pos2;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    if (dist < 2 * radius) {
        sf::Vector2f normal = diff / dist;
        sf::Vector2f relativeVelocity = vel1 - vel2;
        float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

        if (velocityAlongNormal < 0) {
            float restitution = 0.9f; // Coefficient of restitution
            float impulse = -(1 + restitution) * velocityAlongNormal / 2.0f;

            sf::Vector2f impulseVec = impulse * normal;
            vel1 -= impulseVec;
            vel2 += impulseVec;

            // Push balls apart to prevent overlap
            float overlap = 2 * radius - dist;
            pos1 += normal * (overlap / 2.0f);
            pos2 -= normal * (overlap / 2.0f);
        }
    }
}

// Function to spawn debris with explosion-like characteristics
void spawnDebris(std::vector<Debris>& debris, sf::Vector2f position, int count) {
    for (int i = 0; i < count; ++i) {
        Debris d;

        // Random size for varied particle look
        float size = 2.f + (std::rand() % 8); // Size between 2 and 10
        d.shape.setSize(sf::Vector2f(size, size));

        // Color variation (simple example, could be more complex with gradients)
        int colorVariation = std::rand() % 100;
        d.shape.setFillColor(sf::Color(255 - colorVariation, 255 - colorVariation / 2, 0)); // From yellow to orange

        d.shape.setPosition(position);

        // Direction of velocity is radially outward with some randomness
        float angle = (std::rand() % 360) * (3.14159f / 180.0f); // Convert to radians
        float speed = 50.f + (std::rand() % 150); // Speed between 50 and 200
        d.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        // Lifetime variation
        d.lifetime = 0.5f + (std::rand() % 100) / 100.f; // Lifetime between 0.5 and 1.5 seconds

        // Add rotation for dynamic look
        d.shape.setRotation(std::rand() % 360); // Random initial rotation
        d.rotationSpeed = (std::rand() % 100 - 50) / 50.0f; // Rotation speed between -1 and 1

        debris.push_back(d);
    }
}

// Function to update debris
void updateDebris(std::vector<Debris>& debris, float deltaTime) {
    for (auto it = debris.begin(); it != debris.end();) {
        // Update position
        it->shape.move(it->velocity * deltaTime);

        // Apply gravity
        it->velocity.y += 50.f * deltaTime;

        // Reduce lifetime
        it->lifetime -= deltaTime;

        // Particle rotation
        it->shape.rotate(it->rotationSpeed * 360 * deltaTime); // Rotate based on speed

        // Fade out effect
        float alpha = it->shape.getFillColor().a;
        if (alpha > 0) {
            alpha -= 255 * deltaTime / it->lifetime; // Adjust this for speed of fade
            if (alpha < 0) alpha = 0;
            sf::Color newColor = it->shape.getFillColor();
            newColor.a = static_cast<sf::Uint8>(alpha);
            it->shape.setFillColor(newColor);
        }
        if (it->lifetime <= 0) {
            it = debris.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Function to render debris
void renderDebris(sf::RenderWindow& window, const std::vector<Debris>& debris) {
    for (const auto& d : debris) {
        window.draw(d.shape);
    }
}

// Function to reset the level with increased difficulty
void resetLevel(std::vector<sf::RectangleShape>& bricks, const int brickRows, const int brickColumns, const float brickWidth, const float brickHeight, sf::Color brickColors[], float& ballSpeedMultiplier) {
    bricks.clear();
    currentLevel++;
    currentbrickRows++;
    remainingBalls++;
    lastRowFalling = false; // Reset the falling state
    levelClock.restart();  // Restart the timer

    for (int row = 0; row < currentbrickRows; ++row) {
        for (int col = 0; col < brickColumns; ++col) {
            sf::RectangleShape brick(sf::Vector2f(brickWidth, brickHeight));
            brick.setFillColor(brickColors[row % 5]);
            brick.setPosition(10 + col * (brickWidth + 5), 50 + row * (brickHeight + 5));
            bricks.push_back(brick);
        }
    }
    ballSpeedMultiplier += 0.1f; // Slightly increase the ball speed for the new level
}

// Function to display the "YOU SUCK!" message before the game starts
void displayYouSuckMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "YOU SUCK!";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;

    std::vector<sf::RectangleShape> blocks;

    // Create blocks to spell "YOU SUCK!"
    for (size_t i = 0; i < message.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::White);
        block.setPosition(startX + i * (blockSize + 10), startY);
        blocks.push_back(block);
    }

    sf::Text letter;
    letter.setFont(font);
    letter.setCharacterSize(20);
    letter.setFillColor(sf::Color::Black);

    // Animate blocks falling
    for (int frame = 0; frame < 120; ++frame) {
        window.clear();
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i].move(0, 1); // Move blocks down
            window.draw(blocks[i]);

            // Draw letters
            letter.setString(std::string(1, message[i]));
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(5, 5));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

void displayReadyMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "READY?";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;
    const float explosionDuration = 60; // Frames for explosion animation

    std::vector<sf::RectangleShape> blocks;
    std::vector<std::vector<sf::CircleShape>> particles(message.size());

    // Create blocks to spell "READY?"
    for (size_t i = 0; i < message.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::White);
        block.setPosition(startX + i * (blockSize + 10), -blockSize); // Start above the screen
        blocks.push_back(block);

        // Prepare explosion particles for each block
        for (int j = 0; j < 30; ++j) { // 30 particles per block
            sf::CircleShape particle(2); // Small circle for particle effect
            particle.setFillColor(sf::Color::White);
            particle.setPosition(block.getPosition() + sf::Vector2f(blockSize / 2, blockSize / 2));
            particles[i].push_back(particle);
        }
    }

    sf::Text letter;
    letter.setFont(font);
    letter.setCharacterSize(20);
    letter.setFillColor(sf::Color::Black);

    // Animate blocks falling (entrance)
    for (int frame = 0; frame < 120; ++frame) {
        window.clear();
        for (size_t i = 0; i < blocks.size(); ++i) {
            if (blocks[i].getPosition().y < startY) {
                blocks[i].move(0, 3); // Faster fall to simulate gravity
            }
            window.draw(blocks[i]);

            // Draw letters
            letter.setString(std::string(1, message[i]));
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(5, 5));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }

    // Explosion animation
    for (int frame = 0; frame < explosionDuration; ++frame) {
        window.clear();

        for (size_t i = 0; i < blocks.size(); ++i) {
            // Reduce block opacity for fade effect
            blocks[i].setFillColor(sf::Color(255, 255, 255, 255 - (frame * 255 / explosionDuration)));
            window.draw(blocks[i]);

            // Animate particles
            for (auto& particle : particles[i]) {
                particle.setPosition(400, 300);
                float angle = static_cast<float>(rand()) / RAND_MAX * 126.28f; // Random angle in radians
                float speed = static_cast<float>(frame) / explosionDuration * 12.0f; // Increase speed over time
                particle.move(cos(angle) * speed, sin(angle) * speed);
                particle.setFillColor(sf::Color(255, 0, 0, 255 - (frame * 255 / explosionDuration)));
                window.draw(particle);


            }
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

// Function to display the "LEVEL DONE!" message before the game starts
void displayYouWonMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "LEVEL DONE!";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;

    std::vector<sf::RectangleShape> blocks;

    // Create blocks to spell "LEVEL DONE!"
    for (size_t i = 0; i < message.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::White);
        block.setPosition(startX + i * (blockSize + 10), startY);
        blocks.push_back(block);
    }

    sf::Text letter;
    letter.setFont(font);
    letter.setCharacterSize(20);
    letter.setFillColor(sf::Color::Black);

    // Animate blocks falling
    for (int frame = 0; frame < 120; ++frame) {
        window.clear();
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i].move(0, 1); // Move blocks down
            window.draw(blocks[i]);

            // Draw letters
            letter.setString(std::string(1, message[i]));
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(5, 5));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Breakout Remix");
    window.setFramerateLimit(60);

    // Paddle properties
    sf::RectangleShape paddle(sf::Vector2f(200, 20));
    paddle.setFillColor(sf::Color::Green);
    paddle.setPosition(350, 550);

    // Ball properties
    std::vector<sf::CircleShape> balls(1, sf::CircleShape(10));
    std::vector<sf::Vector2f> ballPositions(1, sf::Vector2f(400, 300));
    std::vector<sf::Vector2f> ballVelocities(1, sf::Vector2f(3.0f, -4.0f));
    float ballRadius = 10.0f;
    float ballSpeedMultiplier = 1.0f;

    for (auto& ball : balls) {
        ball.setFillColor(sf::Color::Red);
        ball.setOrigin(10, 10);
    }

    // Bricks
    const int brickColumns = 10;
    const float brickWidth = 60.0f;
    const float brickHeight = 20.0f;
    std::vector<sf::RectangleShape> bricks;
    sf::Color brickColors[] = { sf::Color::Red, sf::Color::Yellow, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta, sf::Color::White, sf::Color::Red, sf::Color::Black };

    resetLevel(bricks, brickRows, brickColumns, brickWidth, brickHeight, brickColors, ballSpeedMultiplier);

    // Score
    int score = 0;
    sf::Font font;
    if (!font.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/font/arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    // Sound effects
    sf::SoundBuffer scoreBuffer, loseBallBuffer, hitBallBuffer, ready3Buffer, winBuffer;
    if (!scoreBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/score.wav") ||
        !loseBallBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/lose_ball.wav") ||
        !hitBallBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/hit_ball.wav") ||
        !ready3Buffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/ready3.wav") ||
        !winBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/win.wav")) {
        std::cerr << "Failed to load sound effects!\n";
        return -1;
    }
    sf::Sound scoreSound(scoreBuffer);
    sf::Sound hitBallSound(hitBallBuffer);
    sf::Sound ready3Sound(ready3Buffer);
    sf::Sound loseBallSound(loseBallBuffer);
    sf::Sound winSound(winBuffer);

    // Display "READY?" and sound at the start
    ready3Sound.play();
    displayReadyMessage(window, font);

    // Game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Paddle movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paddle.getPosition().x > 0) {
            paddle.move(-7.0f, 0.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paddle.getPosition().x + paddle.getSize().x < 800) {
            paddle.move(7.0f, 0.0f);
        }

        // Ball and collision logic
        for (size_t i = 0; i < balls.size(); ++i) {
            ballPositions[i] += ballVelocities[i] * ballSpeedMultiplier;

            // Ball collision with walls
            if (ballPositions[i].x - ballRadius < 0 || ballPositions[i].x + ballRadius > 800) {
                ballVelocities[i].x = -ballVelocities[i].x;
            }
            if (ballPositions[i].y - ballRadius < 0) {
                ballVelocities[i].y = -ballVelocities[i].y;
            }

            // Ball collision with paddle
            if (ballPositions[i].y + ballRadius >= paddle.getPosition().y &&
                ballPositions[i].x + ballRadius >= paddle.getPosition().x &&
                ballPositions[i].x - ballRadius <= paddle.getPosition().x + paddle.getSize().x) {
                ballPositions[i].y = paddle.getPosition().y - ballRadius;
                ballVelocities[i].y = -std::abs(ballVelocities[i].y);
                hitBallSound.play();
            }

            // Ball collision with bricks
            for (auto it = bricks.begin(); it != bricks.end();) {
                if (ballPositions[i].x + ballRadius > it->getPosition().x &&
                    ballPositions[i].x - ballRadius < it->getPosition().x + brickWidth &&
                    ballPositions[i].y + ballRadius > it->getPosition().y &&
                    ballPositions[i].y - ballRadius < it->getPosition().y + brickHeight) {
                    ballVelocities[i].y = -ballVelocities[i].y;
                    it = bricks.erase(it);
                    scoreSound.play();
                    score += 100;
                    spawnDebris(debris, it->getPosition(), brickHitDebris);
                }
                else {
                    ++it;
                }
            }

            // Ball out of bounds
            if (ballPositions[i].y - ballRadius > 600) {
                balls.erase(balls.begin() + i);
                ballPositions.erase(ballPositions.begin() + i);
                ballVelocities.erase(ballVelocities.begin() + i);
                loseBallSound.play();
                --i;

                if (balls.empty() && remainingBalls > 1) {
                    --remainingBalls;
                    balls.push_back(sf::CircleShape(10));
                    balls.back().setFillColor(sf::Color::Red);
                    balls.back().setOrigin(10, 10);
                    ballPositions.push_back(sf::Vector2f(paddle.getPosition().x + paddle.getSize().x / 2, paddle.getPosition().y - 20));
                    ballVelocities.push_back(sf::Vector2f(3.0f, -4.0f));
                }
                else if (balls.empty()) {
                    std::cout << "Game Over!" << std::endl;
                    loseBallSound.play();
                    displayYouSuckMessage(window, font);
                    std::chrono::seconds(3);
                    window.close();
                }

            }
            // Start staggered falling for the last row after 7 seconds
            if (!bricks.empty() && levelClock.getElapsedTime().asSeconds() > 7) {
                lastRowFalling = true; // Start falling after 7 seconds
            }

            if (lastRowFalling) {
                // Ensure `currentBrickIndex` starts at the correct position
                if (currentBrickIndex >= bricks.size()) {
                    currentBrickIndex = bricks.size() - brickColumns;
                    if (currentBrickIndex < 0) currentBrickIndex = 0; // Ensure it doesn't go negative
                }

                // Drop bricks one by one with a delay
                if (brickFallClock.getElapsedTime().asSeconds() > 0.5f && currentBrickIndex < bricks.size()) {
                    bricks[currentBrickIndex].move(0, 50); // Move current brick downward (adjust speed as needed)

                    // Remove brick if it goes out of bounds
                    if (bricks[currentBrickIndex].getPosition().y > 600) {
                        bricks.erase(bricks.begin() + currentBrickIndex);
                        continue; // Skip further processing for this brick
                    }

                    ++currentBrickIndex; // Move to the next brick
                    brickFallClock.restart(); // Reset the clock for the next brick
                }
            }
        }

        // Check if all bricks are cleared
        if (bricks.empty()) {
            winSound.play();
            displayYouWonMessage(window, font);

            resetLevel(bricks, brickRows, brickColumns, brickWidth, brickHeight, brickColors, ballSpeedMultiplier);
        }

        // Update debris before rendering
        sf::Time dt = debrisClock.restart();
        float deltaTime = dt.asSeconds();
        updateDebris(debris, deltaTime);

        // Update ball positions
        for (size_t i = 0; i < balls.size(); ++i) {
            balls[i].setPosition(ballPositions[i]);
        }

        // Update score display
        scoreText.setString("Score: " + std::to_string(score) + " | Balls: " + std::to_string(remainingBalls) + " | Level: " + std::to_string(currentLevel));

        // Render
        window.clear();
        window.draw(paddle);
        for (const auto& brick : bricks) {
            window.draw(brick);
        }
        for (const auto& ball : balls) {
            window.draw(ball);
        }
        renderDebris(window, debris);
        window.draw(scoreText);
        window.display();
    }

    return 0;
}
