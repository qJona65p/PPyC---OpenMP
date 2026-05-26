/*
 * mandelbrot.cpp  –  Versión puramente secuencial hecha con Claude
 *
 * Tarea A: Genera una imagen 8K del Conjunto de Mandelbrot con coloración
 *          suavizada (smooth / continuous colouring) para evitar bandas de
 *          colores abruptas.
 *
 * Tarea B: Aplica un Gaussian Blur separable de radio amplio (radio 15,
 *          kernel de 31 puntos) sobre la imagen generada.  El blur se realiza
 *          sobre los canales RGB ya calculados, no sobre los conteos crudos de
 *          iteración, lo que garantiza que el filtro actúe sobre los mismos
 *          datos que se guardan en disco.
 *
 * Compilación sugerida:
 *   g++ -O2 -std=c++17 -o mandelbrot mandelbrot.cpp
 *
 * Salida:
 *   mandelbrot_original.ppm   (~96 MB en formato P6 binario)
 *   mandelbrot_blurred.ppm    (misma resolución, post-filtro)
 * 
 * Tiempo de ejecución promedio de 21.81592 segundos
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <complex>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

// ─── Parámetros globales ──────────────────────────────────────────────────────

const int WIDTH    = 7680;   // Resolución 8K
const int HEIGHT   = 4320;
const int MAX_ITER = 1000;   // Más iteraciones → más detalle en el borde

const double MIN_REAL = -2.5;
const double MAX_REAL =  1.0;
const double MIN_IMAG = -1.2;
const double MAX_IMAG =  1.2;

// ─── Estructura de imagen RGB ─────────────────────────────────────────────────

struct Pixel { uint8_t r, g, b; };
using Image = vector<vector<Pixel>>;

// ─── Paleta de colores (cyclic, basada en ángulo HSV) ─────────────────────────
//
//  Mapea el valor suavizado t ∈ [0,1) a un color RGB vistoso.
//  Pixels dentro del conjunto (t == 1.0) se pintan de negro.

static Pixel palette(double t) {
    if (t >= 1.0) return {0, 0, 0};          // Dentro del conjunto → negro

    // Tres sinusoides desfasadas → paleta cíclica suave
    const double freq = 6.5 * 3.14159265358979;
    auto ch = [&](double phase) -> uint8_t {
        double v = 0.5 + 0.5 * cos(freq * t + phase);
        return static_cast<uint8_t>(clamp(v * 255.0, 0.0, 255.0));
    };
    return { ch(0.0), ch(2.094), ch(4.189) };   // 0, 2π/3, 4π/3
}

// ─── Tarea A: Generar Mandelbrot ──────────────────────────────────────────────
//
//  Usa la fórmula de "smooth colouring" de Linas Vepstas:
//
//    mu = iter - log2(log2(|z|))
//
//  lo que elimina las discontinuidades entre bandas de iteración.

Image generate_mandelbrot() {
    Image image(HEIGHT, vector<Pixel>(WIDTH));

    auto start = high_resolution_clock::now();

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double cr = MIN_REAL + (MAX_REAL - MIN_REAL) * x / (WIDTH  - 1);
            double ci = MIN_IMAG + (MAX_IMAG - MIN_IMAG) * y / (HEIGHT - 1);

            // Iteración manual (más rápida que std::complex en este bucle)
            double zr = 0.0, zi = 0.0;
            int iter = 0;

            while (zr * zr + zi * zi < 4.0 && iter < MAX_ITER) {
                double tmp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = tmp;
                ++iter;
            }

            double t;
            if (iter == MAX_ITER) {
                t = 1.0;   // Dentro del conjunto
            } else {
                // Smooth colouring: normaliza a [0, 1)
                double log_zn = 0.5 * log(zr * zr + zi * zi);
                double nu     = log(log_zn / log(2.0)) / log(2.0);
                double mu     = iter + 1.0 - nu;
                t = mu / MAX_ITER;
                t = clamp(t, 0.0, 1.0 - 1e-9);
            }

            image[y][x] = palette(t);
        }
    }

    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea A completada en " << elapsed << " s\n";
    return image;
}

// ─── Guardar PPM binario (formato P6) ─────────────────────────────────────────

void save_ppm(const Image& img, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) throw runtime_error("No se pudo abrir: " + filename);

    file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    for (const auto& row : img)
        for (const auto& p : row)
            file.write(reinterpret_cast<const char*>(&p), 3);

    cout << "  Guardado: " << filename << "\n";
}

// ─── Tarea B: Gaussian Blur separable ────────────────────────────────────────
//
//  Radio 15  →  kernel de 31 elementos  →  filtro "pesado" tal como indica
//  el enunciado.  El blur opera sobre los tres canales RGB ya calculados.
//
//  Complejidad: O(WIDTH × HEIGHT × 2 × (2·radius+1)) ≈ 4 000 M operaciones
//  a esta resolución, lo que hace bien visible el tiempo secuencial.

Image gaussian_blur(const Image& src, int radius = 15) {
    // ── Construir kernel 1-D normalizado ──
    double sigma = radius / 3.0;            // sigma = radio/3 → cubre ±3σ
    vector<double> kernel(2 * radius + 1);
    double ksum = 0.0;
    for (int i = -radius; i <= radius; ++i) {
        kernel[i + radius] = exp(-i * i / (2.0 * sigma * sigma));
        ksum += kernel[i + radius];
    }
    for (auto& k : kernel) k /= ksum;

    // ── Buffers intermedios (tres canales separados para claridad) ──
    // Usamos float para acumular sin desbordamiento
    const int N = HEIGHT * WIDTH;
    vector<float> rch(N), gch(N), bch(N);
    vector<float> rout(N), gout(N), bout(N);

    // Separar canales
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            int idx = y * WIDTH + x;
            rch[idx] = src[y][x].r;
            gch[idx] = src[y][x].g;
            bch[idx] = src[y][x].b;
        }

    auto start = high_resolution_clock::now();

    // ── Pase horizontal ──
    vector<float> rh(N), gh(N), bh(N);
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double vr = 0, vg = 0, vb = 0;
            for (int k = -radius; k <= radius; ++k) {
                int nx  = clamp(x + k, 0, WIDTH - 1);
                double w = kernel[k + radius];
                int idx = y * WIDTH + nx;
                vr += rch[idx] * w;
                vg += gch[idx] * w;
                vb += bch[idx] * w;
            }
            int idx = y * WIDTH + x;
            rh[idx] = static_cast<float>(vr);
            gh[idx] = static_cast<float>(vg);
            bh[idx] = static_cast<float>(vb);
        }
    }

    // ── Pase vertical ──
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double vr = 0, vg = 0, vb = 0;
            for (int k = -radius; k <= radius; ++k) {
                int ny  = clamp(y + k, 0, HEIGHT - 1);
                double w = kernel[k + radius];
                int idx = ny * WIDTH + x;
                vr += rh[idx] * w;
                vg += gh[idx] * w;
                vb += bh[idx] * w;
            }
            int idx = y * WIDTH + x;
            rout[idx] = static_cast<float>(vr);
            gout[idx] = static_cast<float>(vg);
            bout[idx] = static_cast<float>(vb);
        }
    }

    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea B completada en " << elapsed << " s  "
         << "(radio=" << radius << ", kernel=" << 2*radius+1 << " pts)\n";

    // ── Reconstruir imagen ──
    Image out(HEIGHT, vector<Pixel>(WIDTH));
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            int idx = y * WIDTH + x;
            out[y][x] = {
                static_cast<uint8_t>(clamp((int)rout[idx], 0, 255)),
                static_cast<uint8_t>(clamp((int)gout[idx], 0, 255)),
                static_cast<uint8_t>(clamp((int)bout[idx], 0, 255))
            };
        }
    return out;
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    auto t0 = high_resolution_clock::now();

    cout << "=== Mandelbrot secuencial  " << WIDTH << "×" << HEIGHT << " ===\n\n";

    cout << "[Tarea A] Generando fractal...\n";
    Image original = generate_mandelbrot();
    save_ppm(original, "mandelbrot_original.ppm");

    cout << "\n[Tarea B] Aplicando Gaussian Blur (radio 15)...\n";
    Image blurred = gaussian_blur(original, 15);
    save_ppm(blurred, "mandelbrot_blurred.ppm");

    double total = duration<double>(high_resolution_clock::now() - t0).count();
    cout << "\nTiempo total: " << total << " s\n";
    return 0;
}
