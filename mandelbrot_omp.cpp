/*
 * mandelbrot_omp.cpp  –  Versión paralela con OpenMP de Claude
 *
 * Tarea A: Genera una imagen 8K del Conjunto de Mandelbrot con coloración
 *          suavizada (smooth colouring).  Cada fila de píxeles se reparte
 *          entre los hilos disponibles con schedule(dynamic) para compensar
 *          el desbalance de carga inherente al fractal (los píxeles del
 *          interior del conjunto requieren MAX_ITER iteraciones; los del
 *          exterior escapan rápido).
 *
 * Tarea B: Aplica un Gaussian Blur separable de radio amplio (radio 15,
 *          kernel de 31 puntos) sobre los canales RGB.  Ambos pases
 *          (horizontal y vertical) se paralelizan por filas con
 *          schedule(static), ya que la carga es uniforme.
 *
 * Compilación sugerida:
 *   g++ -O2 -std=c++17 -fopenmp -o mandelbrot_omp mandelbrot_omp.cpp
 *
 * Control de hilos (ejemplos):
 *   ./mandelbrot_omp                  # usa todos los núcleos disponibles
 *   OMP_NUM_THREADS=8 ./mandelbrot_omp
 *
 * Salida:
 *   mandelbrot_original.ppm   (~96 MB, formato P6 binario)
 *   mandelbrot_blurred.ppm    (misma resolución, post-filtro)
 * 
 * Tiempo de ejecución promedio de 3.767764 segundos
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <stdexcept>
#include <omp.h>          // ← única cabecera extra respecto a la versión secuencial

using namespace std;
using namespace std::chrono;

// ─── Parámetros globales ──────────────────────────────────────────────────────

const int WIDTH    = 7680;
const int HEIGHT   = 4320;
const int MAX_ITER = 1000;

const double MIN_REAL = -2.5;
const double MAX_REAL =  1.0;
const double MIN_IMAG = -1.2;
const double MAX_IMAG =  1.2;

// ─── Estructura de imagen RGB ─────────────────────────────────────────────────

struct Pixel { uint8_t r, g, b; };
using Image = vector<vector<Pixel>>;

// ─── Paleta de colores cíclica (tres cosenos desfasados 120°) ─────────────────
//
//  Función pura → completamente thread-safe, sin efectos secundarios.

static Pixel palette(double t) {
    if (t >= 1.0) return {0, 0, 0};

    const double freq = 6.5 * 3.14159265358979;
    auto ch = [&](double phase) -> uint8_t {
        double v = 0.5 + 0.5 * cos(freq * t + phase);
        return static_cast<uint8_t>(clamp(v * 255.0, 0.0, 255.0));
    };
    return { ch(0.0), ch(2.094), ch(4.189) };
}

// ─── Tarea A: Generar Mandelbrot en paralelo ──────────────────────────────────
//
//  Estrategia de paralelización:
//  • El bucle externo (filas y) se distribuye entre hilos con
//    schedule(static): cada hilo recibe un bloque contiguo de
//    HEIGHT/num_threads filas, asignadas antes de ejecutar.
//  • Ventaja: cero overhead de cola dinámica; acceso a memoria más
//    predecible y amigable con la caché.
//  • Trade-off: no compensa el desbalance de carga inherente al fractal
//    (filas con muchos píxeles interiores tardan más que las del
//    exterior), por lo que algunos hilos pueden quedar ociosos al final.
//  • Todas las variables del bucle interno son locales al hilo → sin
//    condiciones de carrera.  Cada hilo escribe en posiciones distintas
//    de `image`, sin solapamiento.

Image generate_mandelbrot_dynamic(int cs=8) {
    Image image(HEIGHT, vector<Pixel>(WIDTH));
 
    auto start = high_resolution_clock::now();
 
    #pragma omp parallel for schedule(dynamic, cs) default(none) shared(image, cs)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double cr = MIN_REAL + (MAX_REAL - MIN_REAL) * x / (WIDTH  - 1);
            double ci = MIN_IMAG + (MAX_IMAG - MIN_IMAG) * y / (HEIGHT - 1);
 
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
                t = 1.0;
            } else {
                double log_zn = 0.5 * log(zr * zr + zi * zi);
                double nu     = log(log_zn / log(2.0)) / log(2.0);
                double mu     = iter + 1.0 - nu;
                t = clamp(mu / MAX_ITER, 0.0, 1.0 - 1e-9);
            }
 
            image[y][x] = palette(t);
        }
    }
 
    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea A completada en " << elapsed << " s"
         << "  (" << omp_get_max_threads() << " hilos)\n";
    return image;
}

Image generate_mandelbrot_static(int cs) {
    Image image(HEIGHT, vector<Pixel>(WIDTH));
 
    auto start = high_resolution_clock::now();
 
    #pragma omp parallel for schedule(static, cs) default(none) shared(image, cs)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double cr = MIN_REAL + (MAX_REAL - MIN_REAL) * x / (WIDTH  - 1);
            double ci = MIN_IMAG + (MAX_IMAG - MIN_IMAG) * y / (HEIGHT - 1);
 
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
                t = 1.0;
            } else {
                double log_zn = 0.5 * log(zr * zr + zi * zi);
                double nu     = log(log_zn / log(2.0)) / log(2.0);
                double mu     = iter + 1.0 - nu;
                t = clamp(mu / MAX_ITER, 0.0, 1.0 - 1e-9);
            }
 
            image[y][x] = palette(t);
        }
    }
 
    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea A completada en " << elapsed << " s"
         << "  (" << omp_get_max_threads() << " hilos)\n";
    return image;
}

Image generate_mandelbrot_guided(int cs) {
    Image image(HEIGHT, vector<Pixel>(WIDTH));
 
    auto start = high_resolution_clock::now();
 
    #pragma omp parallel for schedule(guided, cs) default(none) shared(image, cs)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double cr = MIN_REAL + (MAX_REAL - MIN_REAL) * x / (WIDTH  - 1);
            double ci = MIN_IMAG + (MAX_IMAG - MIN_IMAG) * y / (HEIGHT - 1);
 
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
                t = 1.0;
            } else {
                double log_zn = 0.5 * log(zr * zr + zi * zi);
                double nu     = log(log_zn / log(2.0)) / log(2.0);
                double mu     = iter + 1.0 - nu;
                t = clamp(mu / MAX_ITER, 0.0, 1.0 - 1e-9);
            }
 
            image[y][x] = palette(t);
        }
    }
 
    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea A completada en " << elapsed << " s"
         << "  (" << omp_get_max_threads() << " hilos)\n";
    return image;
}

// ─── Guardar PPM binario (formato P6) ─────────────────────────────────────────
//
//  La E/S se mantiene secuencial: paralelizar escrituras en un mismo
//  ofstream requeriría sincronización y no aportaría ganancia real
//  (el cuello de botella es el disco, no la CPU).

void save_ppm(const Image& img, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) throw runtime_error("No se pudo abrir: " + filename);

    file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    for (const auto& row : img)
        for (const auto& p : row)
            file.write(reinterpret_cast<const char*>(&p), 3);

    cout << "  Guardado: " << filename << "\n";
}

// ─── Tarea B: Gaussian Blur separable en paralelo ────────────────────────────
//
//  Estrategia de paralelización:
//  • Separación de canales: paralelizada por filas (schedule static,
//    carga perfectamente uniforme).
//  • Pase horizontal: independiente fila a fila → paralelo por filas.
//  • Pase vertical: independiente columna a columna → paralelo también
//    por filas (cada hilo lee de rh/gh/bh y escribe en rout/gout/bout
//    en posiciones distintas, sin solapamiento).
//  • Reconstrucción final: paralela por filas.
//
//  No se necesitan secciones críticas ni reducción: la independencia
//  espacial del blur separable lo hace embarazosamente paralelo.

Image gaussian_blur(const Image& src, int radius = 15) {
    // ── Kernel 1-D normalizado (construido secuencialmente; es O(radius)) ──
    double sigma = radius / 3.0;
    vector<double> kernel(2 * radius + 1);
    double ksum = 0.0;
    for (int i = -radius; i <= radius; ++i) {
        kernel[i + radius] = exp(-i * i / (2.0 * sigma * sigma));
        ksum += kernel[i + radius];
    }
    for (auto& k : kernel) k /= ksum;

    const int N = HEIGHT * WIDTH;
    vector<float> rch(N), gch(N), bch(N);
    vector<float> rh(N),  gh(N),  bh(N);
    vector<float> rout(N), gout(N), bout(N);

    // ── Separar canales (paralelo, sin dependencias) ──
    #pragma omp parallel for schedule(static) default(none) shared(src, rch, gch, bch)
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            int idx = y * WIDTH + x;
            rch[idx] = src[y][x].r;
            gch[idx] = src[y][x].g;
            bch[idx] = src[y][x].b;
        }

    auto start = high_resolution_clock::now();

    // ── Pase horizontal (paralelo por filas) ──
    //
    //  Cada hilo procesa filas independientes; las lecturas de rch/gch/bch
    //  son de solo lectura → sin carreras.  Las escrituras en rh/gh/bh
    //  van a posiciones y*WIDTH+x únicas por hilo.
    #pragma omp parallel for schedule(static) default(none) \
            shared(rch, gch, bch, rh, gh, bh, kernel) firstprivate(radius)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double vr = 0, vg = 0, vb = 0;
            for (int k = -radius; k <= radius; ++k) {
                int nx  = clamp(x + k, 0, WIDTH - 1);
                double w = kernel[k + radius];
                int src_idx = y * WIDTH + nx;
                vr += rch[src_idx] * w;
                vg += gch[src_idx] * w;
                vb += bch[src_idx] * w;
            }
            int idx = y * WIDTH + x;
            rh[idx] = static_cast<float>(vr);
            gh[idx] = static_cast<float>(vg);
            bh[idx] = static_cast<float>(vb);
        }
    }

    // ── Pase vertical (paralelo por filas) ──
    //
    //  Lee de rh/gh/bh (ya completos del pase anterior, barrera implícita
    //  al final del pragma anterior) y escribe en rout/gout/bout.
    #pragma omp parallel for schedule(static) default(none) \
            shared(rh, gh, bh, rout, gout, bout, kernel) firstprivate(radius)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double vr = 0, vg = 0, vb = 0;
            for (int k = -radius; k <= radius; ++k) {
                int ny  = clamp(y + k, 0, HEIGHT - 1);
                double w = kernel[k + radius];
                int src_idx = ny * WIDTH + x;
                vr += rh[src_idx] * w;
                vg += gh[src_idx] * w;
                vb += bh[src_idx] * w;
            }
            int idx = y * WIDTH + x;
            rout[idx] = static_cast<float>(vr);
            gout[idx] = static_cast<float>(vg);
            bout[idx] = static_cast<float>(vb);
        }
    }

    double elapsed = duration<double>(high_resolution_clock::now() - start).count();
    cout << "  Tarea B completada en " << elapsed << " s"
         << "  (radio=" << radius << ", kernel=" << 2*radius+1 << " pts"
         << ", " << omp_get_max_threads() << " hilos)\n";

    // ── Reconstruir imagen (paralelo) ──
    Image out(HEIGHT, vector<Pixel>(WIDTH));
    #pragma omp parallel for schedule(static) default(none) shared(out, rout, gout, bout)
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

void scheduler_chunksize_test(int ceiling = 100) {
    cout << "Probando distintos Chunk Size por scheduler\n";
    for(int cs=1; cs<ceiling; cs*=2) {
        cout << "Chunk Size = " << cs << endl;
        cout << "[Tarea A] Generando fractal scheduler(dynamic)... ";
        generate_mandelbrot_dynamic(cs);

        cout << "[Tarea A] Generando fractal scheduler(static)... ";
        generate_mandelbrot_static(cs);
        
        cout << "[Tarea A] Generando fractal scheduler(guided)... ";
       generate_mandelbrot_guided(cs);
    }
}

int main() {
    auto t0 = high_resolution_clock::now();

    cout << "=== Mandelbrot paralelo (OpenMP)  " << WIDTH << "x" << HEIGHT << " ===\n";
    cout << "    Hilos disponibles: " << omp_get_max_threads() << "\n\n";

    scheduler_chunksize_test(150);
    /*
    cout << "[Tarea A] Generando fractal...\n";
    Image original = generate_mandelbrot_dynamic();
    save_ppm(original, "mandelbrot_original.ppm");
    
    cout << "\n[Tarea B] Aplicando Gaussian Blur (radio 15)...\n";
    Image blurred = gaussian_blur(original, 15);
    save_ppm(blurred, "mandelbrot_blurred.ppm");
    */

    double total = duration<double>(high_resolution_clock::now() - t0).count();
    cout << "\nTiempo total: " << total << " s\n";
    return 0;
}