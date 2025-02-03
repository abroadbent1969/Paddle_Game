#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

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

// Partile function for ball collision
class ParticleSystem {
public:
    struct Particle {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        float lifetime;
    };

    std::vector<Particle> particles;

    void createParticles(sf::Vector2f position, sf::Color color, int count) {
        for (int i = 0; i < count; ++i) {
            Particle p;
            p.shape = sf::CircleShape(2.0f);
            p.shape.setFillColor(color);
            p.shape.setPosition(position);
            p.velocity = sf::Vector2f(
                (std::rand() % 20 - 10) / 10.0f,
                (std::rand() % 20 - 10) / 10.0f
            );
            p.lifetime = 1.0f; // 1 second
            particles.push_back(p);
        }
    }

    void update(float deltaTime) {
        for (auto it = particles.begin(); it != particles.end();) {
            it->lifetime -= deltaTime;
            if (it->lifetime <= 0) {
                it = particles.erase(it);
            }
            else {
                it->shape.move(it->velocity);
                it->velocity *= 0.98f; // Apply damping
                ++it;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& particle : particles) {
            window.draw(particle.shape);
        }
    }
};

// Function to reset the level with increased difficulty
void resetLevel(std::vector<sf::RectangleShape>& bricks, const int brickRows, const int brickColumns, const float brickWidth, const float brickHeight, sf::Color brickColors[], float& ballSpeedMultiplier) {
    bricks.clear();
    for (int row = 0; row < brickRows; ++row) {
        for (int col = 0; col < brickColumns; ++col) {
            sf::RectangleShape brick(sf::Vector2f(brickWidth, brickHeight));
            brick.setFillColor(brickColors[row % 7]);
            brick.setPosition(10 + col * (brickWidth + 5), 50 + row * (brickHeight + 5));
            bricks.push_back(brick);
        }
    }
    ballSpeedMultiplier += 0.1f; // Slightly increase the ball speed for the new level
}

// Function to display the "READY?" message before the game starts
void displayReadyMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "LIZ READY?";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;

    std::vector<sf::RectangleShape> blocks;

    // Create blocks to spell "READY?"
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
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(6, 6));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(100));
    }
}

// Function to display the "YOU WON!" message after level clears
void displayYouWonMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "YOU WON LIZ!";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;

    std::vector<sf::RectangleShape> blocks;

    // Create blocks to spell "YOU WON!"
    for (size_t i = 0; i < message.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::White);
        block.setPosition(startX + i * (blockSize + 20), startY);
        blocks.push_back(block);
    }

    sf::Text letter;
    letter.setFont(font);
    letter.setCharacterSize(25);
    letter.setFillColor(sf::Color::Black);

    // Animate blocks falling
    for (int frame = 0; frame < 150; ++frame) {
        window.clear();
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i].move(0, 1); // Move blocks down
            window.draw(blocks[i]);

            // Draw letters
            letter.setString(std::string(1, message[i]));
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(6, 6));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

// Function to display the "YOU LOSE!" message after level clears
void displayYouLoseMessage(sf::RenderWindow& window, const sf::Font& font) {
    const std::string message = "LIZ YOU LOSE!";
    const float blockSize = 30.0f;
    const float startX = 250.0f;
    const float startY = 250.0f;

    std::vector<sf::RectangleShape> blocks;

    // Create blocks to spell "YOU LOSE!"
    for (size_t i = 0; i < message.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::White);
        block.setPosition(startX + i * (blockSize + 20), startY);
        blocks.push_back(block);
    }

    sf::Text letter;
    letter.setFont(font);
    letter.setCharacterSize(25);
    letter.setFillColor(sf::Color::Black);

    // Animate blocks falling
    for (int frame = 0; frame < 150; ++frame) {
        window.clear();
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i].move(0, 1); // Move blocks down
            window.draw(blocks[i]);

            // Draw letters
            letter.setString(std::string(1, message[i]));
            letter.setPosition(blocks[i].getPosition() + sf::Vector2f(6, 6));
            window.draw(letter);
        }
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Breakout Remix");
    window.setFramerateLimit(60);
    

    // Level properties
    const int Level = 1;
    int myCurrentLevel = Level;
    

    // Paddle properties
    sf::RectangleShape paddle(sf::Vector2f(200, 20));
    paddle.setFillColor(sf::Color::Green);
    paddle.setPosition(350, 550);

    // Ball properties
    const int maxBalls = 3;
    int remainingBalls = maxBalls;
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
    const int brickRows = 5;
    const int brickColumns = 10;
    const float brickWidth = 60.0f;
    const float brickHeight = 20.0f;
    std::vector<sf::RectangleShape> bricks;
    sf::Color brickColors[] = { sf::Color::Red, sf::Color::Yellow, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta };

    resetLevel(bricks, brickRows, brickColumns, brickWidth, brickHeight, brickColors, ballSpeedMultiplier);

    // Level
    int level = 1;
    sf::Font font;
    if (!font.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/font/arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }
    sf::Text levelText;
    levelText.setFont(font);
    levelText.setCharacterSize(20);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition(10, 30);

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
    sf::SoundBuffer scoreBuffer, loseBallBuffer, hitBallBuffer, winBuffer;
    if (!scoreBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/score.wav") ||
        !loseBallBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/lose_ball.wav") ||
        !hitBallBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/hit_ball.wav") ||
        !winBuffer.loadFromFile("C:/Users/abroadbent/source/repos/BMP_Create/wav/win.wav")) {
        std::cerr << "Failed to load sound effects!\n";
        return -1;
    }
    sf::Sound scoreSound(scoreBuffer);
    sf::Sound hitBallSound(hitBallBuffer);
    sf::Sound loseBallSound(loseBallBuffer);
    sf::Sound winSound(winBuffer);

    // Display "READY?" at the start
    displayReadyMessage(window, font);
    winSound.play();

    // Game loop
    while (window.isOpen()) {
            ParticleSystem particleSystem;
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        float deltaTime = 1.0f / 60.0f; // Assuming 60 FPS
        particleSystem.update(deltaTime);
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

                    // Create particles when a brick is hit
                    particleSystem.createParticles(it->getPosition(), it->getFillColor(), 20);

                    it = bricks.erase(it);
                    scoreSound.play();
                    score += 100;
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
                    displayYouLoseMessage(window, font);
                    sf::sleep(sf::milliseconds(100));
                    loseBallSound.play();
                    window.close();
                }
            }
        }

        // Check if all bricks are cleared
        if (bricks.empty()) {
            winSound.play();
            displayYouWonMessage(window, font);
            std::this_thread::sleep_for(std::chrono::seconds(3)); // Pause before resetting
            resetLevel(bricks, brickRows, brickColumns, brickWidth, brickHeight, brickColors, ballSpeedMultiplier);
            ++myCurrentLevel; // Increase level
            balls.push_back(sf::CircleShape(10)); // Reset ball
            ballPositions.push_back(sf::Vector2f(paddle.getPosition().x + paddle.getSize().x / 2, paddle.getPosition().y - 20));
            ballVelocities.push_back(sf::Vector2f(3.0f, -4.0f));
            sf::sleep(sf::milliseconds(100));
        }

        // Update ball positions
        for (size_t i = 0; i < balls.size(); ++i) {
            balls[i].setPosition(ballPositions[i]);
        }

        // Draw particles
        particleSystem.draw(window);

        // Update particles
        //float deltaTime = 1.0f / 60.0f; // Assuming 60 FPS
       // particleSystem.update(deltaTime);

        // Drawing
        window.clear();
        // Draw all game elements...
        window.draw(paddle);
        for (const auto& ball : balls) {
            window.draw(ball);
        }
        for (const auto& brick : bricks) {
            window.draw(brick);
        }

        // Update score display
        scoreText.setString(
            "Score: " + std::to_string(score) +
            " | Level: " + std::to_string(Level) +
            " | Balls: " + std::to_string(remainingBalls)
        );


        // Display score
        //scoreText.setString("Score: " + std::to_string(score));
        //window.draw(scoreText);
        window.display();

    return 0;
}
