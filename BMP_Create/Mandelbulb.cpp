#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>

// Function to map iterations to color (smooth coloring)
sf::Color getColor(int iterations, int max_iterations, double mu) {
    if (iterations == max_iterations) return sf::Color::Black;

    float smooth_value = static_cast<float>(iterations + 1 - std::log(std::log(mu)) / std::log(2.0));
    float hue = 360.0f * smooth_value / max_iterations;

    sf::Uint8 r = static_cast<sf::Uint8>(255 * std::sin(hue));
    sf::Uint8 g = static_cast<sf::Uint8>(255 * std::cos(hue));
    sf::Uint8 b = static_cast<sf::Uint8>(255 * std::sin(2.0 * hue));

    return sf::Color(r, g, b);
}

// Function to calculate Mandelbulb with adjustable complexity (power)
int mandelbulb(float x, float y, float z, int max_iterations, float power) {
    float zx = 0.0f, zy = 0.0f, zz = 0.0f;
    int iterations = 0;

    while (iterations < max_iterations && (zx * zx + zy * zy + zz * zz) < 4.0f) {
        float r = std::sqrt(zx * zx + zy * zy + zz * zz);  // Radius
        float theta = std::atan2(std::sqrt(zx * zx + zy * zy), zz);  // Polar angle
        float phi = std::atan2(zy, zx);  // Azimuthal angle

        // Mandelbulb power transformation (increased power)
        float r_n = std::pow(r, power);  // Increase complexity with a higher power value
        float sin_theta = std::sin(power * theta);
        float cos_theta = std::cos(power * theta);
        float sin_phi = std::sin(power * phi);
        float cos_phi = std::cos(power * phi);

        zx = r_n * sin_theta * cos_phi + x;
        zy = r_n * sin_theta * sin_phi + y;
        zz = r_n * cos_theta + z;

        ++iterations;
    }
    return iterations;
}

int main() {
    const int width = 1920;
    const int height = 1080;
    const int max_iterations = 1000;

    sf::RenderWindow window(sf::VideoMode(width, height), "Complex Mandelbulb Fractal");

    // Image to store pixel data
    sf::Image fractalImage;
    fractalImage.create(width, height, sf::Color::Black);

    // Initial camera settings
    float zoomFactor = 0.5f;  // Start zoomed out more to better frame the Mandelbulb
    float min_x = -1.5f, max_x = 1.5f;
    float min_y = -1.5f, max_y = 1.5f;
    float min_z = -1.5f, max_z = 1.5f;

    // Camera position controls
    float move_speed = 0.2f;  // Increased movement speed
    float zoom_speed = 1.1f;  // Increased zoom speed
    float offset_x = 0.0f, offset_y = 0.0f, offset_z = -2.0f;  // Start camera pulled back a little on Z-axis

    // Complexity control: Mandelbulb power (adjust for more intricate shapes)
    float power = 10.0f;  // Increased power to make the shape more complex

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Camera controls
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            offset_x -= move_speed / zoomFactor;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            offset_x += move_speed / zoomFactor;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            offset_y += move_speed / zoomFactor;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            offset_y -= move_speed / zoomFactor;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            offset_z += move_speed / zoomFactor;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            offset_z -= move_speed / zoomFactor;
        }

        // Zooming with keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            zoomFactor *= zoom_speed;  // Zoom in
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
            zoomFactor /= zoom_speed;  // Zoom out
        }

        // Recalculate the bounds of the 3D space based on zoom and position
        float real_range_x = (max_x - min_x) / zoomFactor;
        float real_range_y = (max_y - min_y) / zoomFactor;
        float real_range_z = (max_z - min_z) / zoomFactor;

        float current_min_x = offset_x - real_range_x / 2;
        float current_max_x = offset_x + real_range_x / 2;
        float current_min_y = offset_y - real_range_y / 2;
        float current_max_y = offset_y + real_range_y / 2;
        float current_min_z = offset_z - real_range_z / 2;
        float current_max_z = offset_z + real_range_z / 2;

        // Generate the Mandelbulb fractal for the current camera view
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Map pixel to 3D space
                float zx = current_min_x + (x * real_range_x) / width;
                float zy = current_min_y + (y * real_range_y) / height;
                float zz = offset_z;  // This helps render a slice of the Mandelbulb

                // Compute Mandelbulb iteration
                int iterations = mandelbulb(zx, zy, zz, max_iterations, power);

                // Smooth coloring
                sf::Color color = getColor(iterations, max_iterations, zx);
                fractalImage.setPixel(x, y, color);
            }
        }

        // Load fractal into a texture
        sf::Texture texture;
        texture.loadFromImage(fractalImage);

        // Sprite to display the fractal
        sf::Sprite sprite(texture);

        // Main loop to display the fractal
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
