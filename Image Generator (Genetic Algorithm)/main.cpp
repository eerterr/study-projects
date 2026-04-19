#include <bits/stdc++.h>
using namespace std;

// stb headers
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

// CONFIGURATION PARAMETERS

// Input and output paths.
const char* INPUT_IMAGE_PATH    = "C:\\Users\\eudok\\CLionProjects\\itai_ass_22\\target_ims\\input5.jpg"; // absolute path to the target image.
const char* OUTPUT_BEST_PATH    = "best_block_pixel_final.png";                                           // final best image after evolution (upscaled).
const char* OUTPUT_LOG_PREFIX   = "gen_";                                                                 // prefix for optional per-generation image dumps.
const char* OUTPUT_INITIAL_PATH = "initial_random_state.png";                                             // snapshot of the initial random individual.
const char* LOG_FILE_PATH       = "logs.txt";                                                             // CSV file storing per-generation statistics.

// Global wall-clock time limit for the entire GA run, in milliseconds.
const int   MAX_TIME_MS         = 570000;                                                                 // 580 seconds (~9.7 minutes).

const int   POP_SIZE            = 100;                                                                    // number of individuals in the population.

// Block Resolution Settings
const int   BLOCK_RESOLUTION    = 48;                                                                     // working GA image resolution (48x48 blocks).
const int   MAX_GENERATIONS     = 4800;                                                                   // hard cap on the number of generations.
const double TARGET_SSIM        = 0.90;                                                                   // early-stopping threshold on SSIM.

// Selection and reproduction parameters.
const int    TOURNAMENT_SIZE    = 8;                                                                      // individuals per tournament (selection pressure).
const double CROSSOVER_RATE     = 0.7;                                                                    // probability of applying one-point crossover.
const double MUTATION_RATE      = 0.7;                                                                    // probability of mutating an offspring individual.

// Stochastic Mutation Settings
const double BLOCK_COLOR_SIGMA  = 20.0;                                                                   // std dev of Gaussian color noise (mutation step size).

// Dynamic Mutation Settings for Stagnation Control (Adaptation)
const int    STAGNATION_THRESHOLD      = 5;                                                               // generations without improvement before adaptation.
const double BLOCK_MUTATE_RATE_INITIAL = 0.01;                                                            // initial probability that a given block in an individual mutates.
const double BLOCK_MUTATE_RATE_BOOST   = 0.05;                                                            // increment added to per-block mutation rate on stagnation.
const double BLOCK_MUTATE_RATE_MAX     = 0.7;                                                             // upper bound for per-block mutation probability.

// SSIM and fitness weighting.
const int    SSIM_WINDOW        = 8;                                                                      // window size (8x8) for local SSIM-like computation.
const double LAMBDA_SSIM        = 0.5;                                                                    // weight of SSIM component in the fitness F.

// Generation interval for saving intermediate best images.
const int    SAVE_EVERY_N_GEN   = 100;                                                                    // save best individual every N generations (optional).

// Random engine used throughout the algorithm for all stochastic decisions:
// selection (tournaments), crossover point, and mutation sampling.
std::mt19937_64 rng(123456789);

// BASIC IMAGE STRUCT & UTILS

// Simple float-based image container.
// data is stored in row-major order with interleaved RGB channels.
struct Image {
    int w = 0, h = 0, c = 0;
    vector<float> data; // range [0..255], size = w*h*c
};

// Helper to convert (x, y, channel) into a linear index in an image buffer
inline int idx(int x, int y, int ch, int W, int C) {
    return (y*W + x)*C + ch;
}

// Load an image from disk into the Image struct using stb_image.
// All images are converted to 3 channels (RGB) and stored as floats in [0, 255]
bool load_image(const char* path, Image &img) {
    int w,h,n;
    unsigned char* pixels = stbi_load(path, &w, &h, &n, 3);
    if (!pixels) {
        cerr << "Failed to load image: " << path << "\n";
        return false;
    }
    img.w = w;
    img.h = h;
    img.c = 3;
    img.data.assign(w*h*3, 0.0f);
    for (int i = 0; i < w*h*3; ++i) {
        img.data[i] = static_cast<float>(pixels[i]);
    }
    stbi_image_free(pixels);
    cerr << "Loaded image: " << path << " (" << w << "x" << h << ", 3 channels)\n";
    return true;
}

// Save an Image to disk in PNG format using stb_image_write.
// The internal float buffer [0,255] is clamped and converted to unsigned char.
void save_image(const char* path, const Image &img) {
    vector<unsigned char> out(img.data.size());
    for (size_t i = 0; i < img.data.size(); ++i) {
        float v = img.data[i];
        if (v < 0.0f) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        out[i] = static_cast<unsigned char>(v + 0.5f);
    }
    stbi_write_png(path, img.w, img.h, img.c, out.data(), img.w*img.c);
}

// Downsample an arbitrary input image to a new_size x new_size square image using bilinear sampling.
// This is used to obtain the 48x48 "target" against which the GA evaluates fitness.
Image downsample_to_square(const Image &src, int new_size) {
    Image dst; dst.w = new_size; dst.h = new_size; dst.c = src.c;
    dst.data.assign(dst.w * dst.h * dst.c, 0.0f);
    float scale_x = (float)src.w / (float)new_size;
    float scale_y = (float)src.h / (float)new_size;

    for (int y = 0; y < new_size; ++y) {
        for (int x = 0; x < new_size; ++x) {
            float src_x = (x + 0.5f) * scale_x - 0.5f;
            float src_y = (y + 0.5f) * scale_y - 0.5f;

            int x0 = (int)floor(src_x), y0 = (int)floor(src_y);
            int x1 = x0 + 1, y1 = y0 + 1;
            float fx = src_x - x0, fy = src_y - y0;

            if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
            if (x1 >= src.w) x1 = src.w - 1; if (y1 >= src.h) y1 = src.h - 1;
            if (x0 >= src.w) x0 = src.w - 1; if (y0 >= src.h) y0 = src.h - 1;

            for (int ch = 0; ch < dst.c; ++ch) {
                float c00 = src.data[idx(x0,y0,ch,src.w,src.c)];
                float c10 = src.data[idx(x1,y0,ch,src.w,src.c)];
                float c01 = src.data[idx(x0,y1,ch,src.w,src.c)];
                float c11 = src.data[idx(x1,y1,ch,src.w,src.c)];

                float c0 = c00*(1.0f-fx) + c10*fx;
                float c1 = c01*(1.0f-fy) + c11*fy;
                float c  = c0*(1.0f-fy) + c1*fy;
                dst.data[idx(x,y,ch,dst.w,dst.c)] = c;
            }
        }
    }
    cerr << "Downsampled target to " << new_size << "x" << new_size << "\n";
    return dst;
}

// Upscale a low-resolution image (e.g., 48x48) to the target resolution using nearest neighbor.
// This is used to produce 512x512 (or original-size) visualizations of individuals.
Image upscale_to_match(const Image &src, int targetW, int targetH) {
    Image dst; dst.w = targetW; dst.h = targetH; dst.c = src.c;
    dst.data.assign(dst.w * dst.h * dst.c, 0.0f);

    float scale_x = (float)src.w / (float)targetW;
    float scale_y = (float)src.h / (float)targetH;

    for (int y = 0; y < targetH; ++y) {
        for (int x = 0; x < targetW; ++x) {
            int src_x = (int)floor(x * scale_x);
            int src_y = (int)floor(y * scale_y);

            src_x = min(src_x, src.w - 1);
            src_y = min(src_y, src.h - 1);

            int src_base = idx(src_x, src_y, 0, src.w, src.c);
            int dst_base = idx(x, y, 0, dst.w, dst.c);

            for (int ch = 0; ch < dst.c; ++ch) {
                dst.data[dst_base + ch] = src.data[src_base + ch];
            }
        }
    }
    return dst;
}


//мBLOCK-BASED GENOME & FITNESS

// FitnessResult aggregates mean squared error (MSE), SSIM-like similarity,
// and the combined scalar fitness F that is minimized by the GA.
struct FitnessResult {
    double mse;
    double ssim;
    double F;
};

// Individual encodes a 48x48 RGB mosaic as a flat color vector (block_colors).
// render holds the corresponding Image representation; render_valid/fit_valid
// allow lazy recomputation of render and fitness.
struct Individual {
    vector<float> block_colors;
    Image render;
    FitnessResult fit;
    bool render_valid = false;
    bool fit_valid    = false;
};

// Covariance helper for the 3D color vectors in SSIM computation.
struct Cov3 { double m[9]; };

// Compute mean squared error between two images interpreted as vectors of RGB values.
// Both images must have the same dimensions and 3 channels.
inline double compute_vector_mse(const Image &a, const Image &b) {
    int N = a.w * a.h; int C = a.c; double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        for (int ch = 0; ch < C; ++ch) {
            double diff = double(a.data[i*C + ch]) - double(b.data[i*C + ch]);
            sum += diff*diff;
        }
    }
    return sum / double(N);
}

// Compute a simplified SSIM-like metric between two color images.
// The implementation operates on 3D color vectors in non-overlapping ws x ws windows
// and uses a luminance term plus a trace-based covariance term to approximate structural similarity.
double compute_vector_ssim_like(const Image &x, const Image &y) {
    int W = x.w, H = x.h, C = x.c; int ws = SSIM_WINDOW; if (W < ws || H < ws) ws = min(W,H);
    const double C1 = 0.01 * 0.01 * 255.0 * 255.0; const double C2 = 0.03 * 0.03 * 255.0 * 255.0;
    const double C3 = C2 / 2.0; const double eps = 1e-8; double sum_ssim = 0.0; int count = 0;

    for (int by = 0; by + ws <= H; by += ws) {
        for (int bx = 0; bx + ws <= W; bx += ws) {
            double mu_x[3] = {0,0,0}, mu_y[3] = {0,0,0}; int N = ws*ws;
            // Compute mean color vectors over the current window.
            for (int dy = 0; dy < ws; ++dy) {
                for (int dx = 0; dx < ws; ++dx) {
                    int px = bx + dx, py = by + dy; int base = idx(px,py,0,W,C);
                    for (int ch=0; ch<3; ++ch) { mu_x[ch] += x.data[base+ch]; mu_y[ch] += y.data[base+ch]; }
                }
            }
            for (int ch=0; ch<3; ++ch) { mu_x[ch] /= double(N); mu_y[ch] /= double(N); }

            Cov3 Sxy; for(int i=0;i<9;++i) { Sxy.m[i]=0.0; }

            // Accumulate cross-covariance matrix between color deviations in x and y.
            for (int dy = 0; dy < ws; ++dy) {
                for (int dx = 0; dx < ws; ++dx) {
                    int px = bx + dx, py = by + dy; int base = idx(px,py,0,W,C);
                    double dxv[3], dyv[3];
                    for (int ch=0; ch<3; ++ch) {
                        dxv[ch] = x.data[base+ch] - mu_x[ch];
                        dyv[ch] = y.data[base+ch] - mu_y[ch];
                    }
                    Sxy.m[0] += dxv[0]*dyv[0]; Sxy.m[1] += dxv[0]*dyv[1]; Sxy.m[2] += dxv[0]*dyv[2];
                    Sxy.m[3] += dxv[1]*dyv[0]; Sxy.m[4] += dxv[1]*dyv[1]; Sxy.m[5] += dxv[1]*dyv[2];
                    Sxy.m[6] += dxv[2]*dyv[0]; Sxy.m[7] += dxv[2]*dyv[1]; Sxy.m[8] += dxv[2]*dyv[2];
                }
            }
            double normN = (N>1)?(1.0/double(N-1)):1.0;
            for (int i=0;i<9;++i) { Sxy.m[i]*=normN; }

            double mu_x_norm2 = mu_x[0]*mu_x[0]+mu_x[1]*mu_x[1]+mu_x[2]*mu_x[2];
            double mu_y_norm2 = mu_y[0]*mu_y[0]+mu_y[1]*mu_y[1]+mu_y[2]*mu_y[2];
            double mu_x_norm  = sqrt(mu_x_norm2+eps); double mu_y_norm  = sqrt(mu_y_norm2+eps);

            // Luminance term based on the norms of the mean color vectors.
            double l = (2.0*mu_x_norm*mu_y_norm + C1) / (mu_x_norm2 + mu_y_norm2 + C1);
            // Use trace of covariance as a simple measure of structural correlation.
            double tr_xy = Sxy.m[0]+Sxy.m[4]+Sxy.m[8];

            // Simplified local similarity measure preserving the idea of window-based comparison.
            double ssim_win = l * max(0.0, tr_xy / (100.0 * 100.0 * 3.0 + eps));
            if (ssim_win>1.0) ssim_win=1.0; if (ssim_win<0.0) ssim_win=0.0;
            sum_ssim += ssim_win; ++count;
        }
    }
    if (count==0) return 0.0;
    double ssim_global = sum_ssim / double(count);
    if (ssim_global<0.0) ssim_global=0.0; if (ssim_global>1.0) ssim_global=1.0; return ssim_global;
}

// Compute and cache fitness for a given individual relative to the target image.
// F is minimized and combines pixel-wise error (MSE) with structural similarity (SSIM).
FitnessResult compute_fitness(Individual &ind, const Image &target) {
    if (!ind.render_valid) {
        ind.render.w = BLOCK_RESOLUTION; ind.render.h = BLOCK_RESOLUTION; ind.render.c = 3;
        ind.render.data = ind.block_colors;
        ind.render_valid = true;
    }
    FitnessResult fr;
    fr.mse  = compute_vector_mse(ind.render, target);
    fr.ssim = compute_vector_ssim_like(ind.render, target);

    // Composite fitness to be minimized.
    fr.F    = fr.mse + LAMBDA_SSIM*(1.0 - fr.ssim);
    ind.fit = fr;
    ind.fit_valid = true;
    return fr;
}
// RENDER & GA OPERATORS

// Render an individual's genome (block_colors) into an Image object.
// This is a direct view: the 48x48x3 buffer is simply interpreted as an image.
void render_individual(const Individual &ind, Image &canvas) {
    canvas.w = BLOCK_RESOLUTION; canvas.h = BLOCK_RESOLUTION; canvas.c = 3;
    canvas.data = ind.block_colors;
}

// Tournament selection: randomly sample TOURNAMENT_SIZE individuals
// and return the index of the one with the best (lowest) fitness F.
int tournament_select(const vector<Individual> &pop) {
    uniform_int_distribution<int> dist(0, (int)pop.size()-1);
    int best = dist(rng);
    for (int i=1;i<TOURNAMENT_SIZE;++i) {
        int idx = dist(rng);
        if (pop[idx].fit.F < pop[best].fit.F) best = idx;
    }
    return best;
}

// One-point crossover on the flattened color vector.
// Parents p1 and p2 are combined into children c1 and c2 by swapping a suffix
// starting at a randomly chosen cut point.
void crossover(const Individual &p1, const Individual &p2, Individual &c1, Individual &c2) {
    c1 = p1; c2 = p2;
    int n = (int)p1.block_colors.size();
    if (n < 2) return;
    uniform_int_distribution<int> cutDist(1, n-1);
    int cut = cutDist(rng);
    for (int i = cut; i < n; ++i) {
        std::swap(c1.block_colors[i], c2.block_colors[i]);
    }
    c1.render_valid = c1.fit_valid = false; c2.render_valid = c2.fit_valid = false;
}

// Purely stochastic mutation: apply Gaussian noise to block colors
// with a per-block probability (block_mutation_rate).
// No gradient or target-dependent component is used here; mutation is unbiased noise.
void mutate(Individual &ind, double block_mutation_rate) {
    uniform_real_distribution<double> prob(0.0,1.0);
    normal_distribution<double> gauss_color(0.0, BLOCK_COLOR_SIGMA);

    int N_blocks = ind.block_colors.size() / 3;

    for (int i = 0; i < N_blocks; ++i) {
        if (prob(rng) < block_mutation_rate) {
            int base = i * 3;

            for (int ch = 0; ch < 3; ++ch) {
                float mutation_step = (float)gauss_color(rng);

                // Apply mutation step and clamp to valid [0, 255] color range.
                ind.block_colors[base + ch] += mutation_step;
                ind.block_colors[base + ch] = min(255.0f, max(0.0f, ind.block_colors[base + ch]));
            }
        }
    }
    ind.render_valid = ind.fit_valid = false;
}

// Initialize an individual with independent random RGB values for each block.
// This creates a high-entropy starting population of noisy mosaics.
void init_random_individual(Individual &ind) {
    int n = BLOCK_RESOLUTION * BLOCK_RESOLUTION; // Number of blocks (pixels in working resolution)
    ind.block_colors.assign(n * 3, 0.0f);

    // Uniform random colors in [0, 255] for each channel.
    uniform_real_distribution<float> colDist(0.0f, 255.0f);

    for (int y = 0; y < BLOCK_RESOLUTION; ++y) {
        for (int x = 0; x < BLOCK_RESOLUTION; ++x) {
            int base_color = idx(x, y, 0, BLOCK_RESOLUTION, 3);

            ind.block_colors[base_color + 0] = colDist(rng);
            ind.block_colors[base_color + 1] = colDist(rng);
            ind.block_colors[base_color + 2] = colDist(rng);
        }
    }
    ind.render_valid = ind.fit_valid = false;
}

// GA LOOP (WITH ADAPTIVE MUTATION AND LOGGING)

// GAResult aggregates outcomes of a GA run: the best individual, its fitness values,
// the number of generations processed, and a flag indicating whether TARGET_SSIM was reached.
struct GAResult {
    Individual best;
    double bestF;
    double bestSSIM;
    int generations;
    bool reachedTarget;
};

// Main GA loop: evolves the population using tournament selection, crossover, mutation,
// and an adaptive per-block mutation schedule to escape stagnation. It also:
// - logs fitness statistics per generation to a CSV file,
// - respects a global wall-clock time limit,
// - optionally stops early when TARGET_SSIM is reached.
GAResult run_ga(vector<Individual> &population,
                const Image &target_block_res,
                int maxGenerations,
                double targetSSIM,
                std::ofstream& log_file)
{
    using clock = std::chrono::steady_clock;
    auto start_time = clock::now();

    std::uniform_real_distribution<double> prob(0.0,1.0);

    double global_bestF = numeric_limits<double>::infinity();
    double global_bestSSIM = 0.0;
    int global_best_idx = 0;

    // Write CSV header for offline analysis of convergence behavior.
    log_file << "Gen,Best_F,Best_SSIM,Mut_Rate\n";

    // Initial fitness evaluation for all individuals.
    for (int i=0;i<POP_SIZE;++i) {
        compute_fitness(population[i], target_block_res);
        if (population[i].fit.F < global_bestF) {
            global_bestF = population[i].fit.F;
            global_bestSSIM = population[i].fit.ssim;
            global_best_idx = i;
        }
    }

    // State variables for adaptive mutation (stagnation detection).
    int generations_since_last_improvement = 0;
    double current_block_mutation_rate = BLOCK_MUTATE_RATE_INITIAL;
    double best_F_in_phase = global_bestF;
    const double EPSILON = 1e-6;

    cerr << "GA start: bestF=" << fixed << setprecision(4) << global_bestF
         << " bestSSIM=" << fixed << setprecision(4) << global_bestSSIM
         << " blocks=" << BLOCK_RESOLUTION*BLOCK_RESOLUTION << "\n";

    // Log initial state (generation 0) to CSV.
    log_file << 0 << ","
             << fixed << setprecision(4) << global_bestF << ","
             << fixed << setprecision(4) << global_bestSSIM << ","
             << fixed << setprecision(3) << current_block_mutation_rate << "\n";

    for (int gen=1; gen<=maxGenerations; ++gen) {

        // Global wall-clock time check: stop if time limit is exceeded.
        auto now = clock::now();
        int elapsed_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        if (elapsed_ms >= MAX_TIME_MS) {
            cerr << "Time limit reached at generation " << gen << "\n";
            break;
        }

        // Adaptive mutation logic based on stagnation in best fitness
        if (global_bestF < best_F_in_phase - EPSILON) {
            // New improvement: reset stagnation counter and restore base mutation rate.
            best_F_in_phase = global_bestF;
            generations_since_last_improvement = 0;
            current_block_mutation_rate = BLOCK_MUTATE_RATE_INITIAL;
        } else {
            // No significant improvement this generation.
            generations_since_last_improvement++;
        }

        if (generations_since_last_improvement >= STAGNATION_THRESHOLD) {
            // Increase per-block mutation rate to escape local minima, up to a safe maximum.
            double new_rate = current_block_mutation_rate + BLOCK_MUTATE_RATE_BOOST;
            current_block_mutation_rate = min(BLOCK_MUTATE_RATE_MAX, new_rate);

            generations_since_last_improvement = 0;

            cerr << "!!! STAGNATION detected (Gen " << gen << "). Boosting MUTATE_RATE to "
                 << fixed << setprecision(3) << current_block_mutation_rate << " !!!\n";
        }

        // Print concise diagnostics to stderr for real-time monitoring.
        cerr << "Gen " << gen
             << " bestF=" << fixed << setprecision(4) << global_bestF
             << " bestSSIM=" << fixed << setprecision(4) << global_bestSSIM
             << " mut_rate=" << fixed << setprecision(3) << current_block_mutation_rate
             << "\n";

        // Log current generation statistics to CSV for later plotting and analysis.
        log_file << gen << ","
                 << fixed << setprecision(4) << global_bestF << ","
                 << fixed << setprecision(4) << global_bestSSIM << ","
                 << fixed << setprecision(3) << current_block_mutation_rate << "\n";


        // Early stopping if target SSIM is achieved.
        if (global_bestSSIM >= targetSSIM) {
            cerr << "Target SSIM reached.\n";
            // Save the best individual at the point of convergence.
            Image bestRender;
            render_individual(population[global_best_idx], bestRender);
            Image final_render = upscale_to_match(bestRender, target_block_res.w, target_block_res.h);
            string path = string(OUTPUT_LOG_PREFIX) + to_string(gen) + ".png";
            save_image(path.c_str(), final_render);
            break;
        }

        // Optional periodic dump of the best individual for visual inspection of convergence.
        if (gen % SAVE_EVERY_N_GEN == 0) {
            Image bestRender;
            render_individual(population[global_best_idx], bestRender);
            Image final_render = upscale_to_match(bestRender, target_block_res.w, target_block_res.h);
            string path = string(OUTPUT_LOG_PREFIX) + to_string(gen) + ".png";
            save_image(path.c_str(), final_render);
        }

        // Construct next generation:
        // - Elitism: preserve current global best as newPop[0].
        // - Fill the rest via tournament selection, crossover, and mutation.
        vector<Individual> newPop(POP_SIZE);
        newPop[0] = population[global_best_idx];

        for (int i = 1; i < POP_SIZE; i += 2) {
            int p1_idx = tournament_select(population);
            int p2_idx = tournament_select(population);

            Individual child1 = population[p1_idx];
            Individual child2 = population[p2_idx];

            if (prob(rng) < CROSSOVER_RATE) {
                crossover(population[p1_idx], population[p2_idx], child1, child2);
            }

            if (prob(rng) < MUTATION_RATE) {
                mutate(child1, current_block_mutation_rate);
            }
            if (i+1 < POP_SIZE && prob(rng) < MUTATION_RATE) {
                mutate(child2, current_block_mutation_rate);
            }

            newPop[i] = std::move(child1);
            if (i+1 < POP_SIZE) {
                newPop[i+1] = std::move(child2);
            }
        }

        population.swap(newPop);

        // Recompute fitness for all individuals and update global best statistics.
        global_bestF = numeric_limits<double>::infinity();
        global_bestSSIM = 0.0;
        global_best_idx = 0;
        for (int i=0;i<POP_SIZE;++i) {
            compute_fitness(population[i], target_block_res);
            if (population[i].fit.F < global_bestF) {
                global_bestF = population[i].fit.F;
                global_bestSSIM = population[i].fit.ssim;
                global_best_idx = i;
            }
        }
    }

    GAResult res;
    res.best = population[global_best_idx];
    res.bestF = global_bestF;
    res.bestSSIM = global_bestSSIM;
    res.generations = maxGenerations;        // Note: may stop earlier due to time/SSIM, but this field keeps the nominal cap.
    res.reachedTarget = (global_bestSSIM >= targetSSIM);

    return res;
}

// MAIN

int main() {
    cout << fixed << setprecision(4);
    cerr << fixed << setprecision(4);
    ios::sync_with_stdio(false);

    // Load the original high-resolution target image.
    Image target_full;
    if (!load_image(INPUT_IMAGE_PATH, target_full)) {
        return 1;
    }

    // Downsample the target to the working GA resolution (BLOCK_RESOLUTION x BLOCK_RESOLUTION).
    Image target_block_res = downsample_to_square(target_full, BLOCK_RESOLUTION);

    // Initialize the population with purely random individuals.
    vector<Individual> population(POP_SIZE);
    for (int i=0;i<POP_SIZE;++i) {
        init_random_individual(population[i]);
    }

    // Save a sample of the initial random individual, upscaled to the original resolution,
    // for qualitative comparison against evolved results.
    Image initialRender_block_res;
    render_individual(population[0], initialRender_block_res);
    Image initial_output = upscale_to_match(initialRender_block_res, target_full.w, target_full.h);
    save_image(OUTPUT_INITIAL_PATH, initial_output);
    cerr << "Saved initial random state (pure noise) to " << OUTPUT_INITIAL_PATH << "\n";

    // Open log file for recording fitness and mutation statistics across generations.
    std::ofstream log_file(LOG_FILE_PATH);
    if (!log_file.is_open()) {
        cerr << "FATAL: Could not open log file: " << LOG_FILE_PATH << "\n";
        return 1;
    }
    cerr << "Logging fitness data to " << LOG_FILE_PATH << "\n";

    // Execute the genetic algorithm until time, generation, or SSIM limits are reached.
    GAResult res = run_ga(population, target_block_res, MAX_GENERATIONS, TARGET_SSIM, log_file);

    // Close the log file explicitly after GA completion.
    log_file.close();

    // Post-processing: save the top-N elites from the final generation for analysis

    // Sort final population in ascending order of F (best individuals first).
    std::sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fit.F < b.fit.F;
    });

    // Save up to 5 best final individuals, upscaled to the original resolution,
    // to inspect diversity among top-ranked solutions.
    int num_to_save = std::min((int)population.size(), 5);
    for (int i = 0; i < num_to_save; ++i) {
        Image current_block_res;
        render_individual(population[i], current_block_res);

        Image final_output = upscale_to_match(current_block_res, target_full.w, target_full.h);

        string path = "best_final_rank_" + to_string(i + 1) + ".png";
        save_image(path.c_str(), final_output);

        cerr << "Saved rank " << i + 1
             << " (F=" << fixed << setprecision(4) << population[i].fit.F
             << ", SSIM=" << fixed << setprecision(4) << population[i].fit.ssim
             << ") to " << path << "\n";
    }
    // End of additional saving of final generation elites

    // Save the single best final individual to the standard output file
    // required by the assignment workflow.
    Individual& best_final = population[0];

    Image bestRender_block_res;
    render_individual(best_final, bestRender_block_res);

    Image final_output = upscale_to_match(bestRender_block_res, target_full.w, target_full.h);
    save_image(OUTPUT_BEST_PATH, final_output);

    cerr << "\nFinished. Final best SSIM=" << best_final.fit.ssim
         << ", F=" << best_final.fit.F
         << ". Saved final best to " << OUTPUT_BEST_PATH << "\n";

    return 0;
}
