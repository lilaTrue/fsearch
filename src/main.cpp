#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <numeric>
#include <map>
#include <algorithm>

// --- ANSI Color Codes ---
#define ANSI_COLOR_RESET   L"\x1b[0m"
#define ANSI_COLOR_YELLOW  L"\x1b[33m"
#define ANSI_COLOR_CYAN    L"\x1b[36m"

// Mutex et CV uniquement pour la file d'attente des répertoires
std::mutex queue_mtx;
std::condition_variable work_cv;
std::queue<std::wstring> directories_to_search;

// Variables de contrôle atomiques
std::atomic<bool> done = false;
std::atomic<long long> tasks_in_progress = 0;
std::atomic<bool> searching_animation = true;
std::wstring search_filename;

// Stockage des résultats local à chaque thread (map: directory -> vector of filenames)
thread_local std::map<std::wstring, std::vector<std::wstring>> local_found_map;

// Vecteur global pour collecter les maps locales de chaque thread à la fin
std::vector<std::map<std::wstring, std::vector<std::wstring>>> all_thread_results;
std::mutex all_thread_results_mtx; // Mutex pour protéger la collecte finale

void enable_virtual_terminal_processing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(hOut, dwMode)) return;
}

void show_animation() {
    const wchar_t animation[] = L"|/-\\";
    int i = 0;
    while (searching_animation) {
        std::wcout << L"\r" << ANSI_COLOR_YELLOW << "Scanning... " << animation[i++ % 4] << ANSI_COLOR_RESET << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::wcout << L"\rScan complete!                                " << std::endl;
}

void search_worker() {
    while (true) {
        std::wstring current_dir_str;
        {
            std::unique_lock<std::mutex> lock(queue_mtx);
            work_cv.wait(lock, [] { return !directories_to_search.empty() || done; });

            if (done && directories_to_search.empty()) {
                break;
            }

            current_dir_str = directories_to_search.front();
            directories_to_search.pop();
        }

        WIN32_FIND_DATAW find_file_data;
        std::wstring search_path = current_dir_str + L"\\*";
        HANDLE h_find = FindFirstFileW(search_path.c_str(), &find_file_data);

        if (h_find != INVALID_HANDLE_VALUE) {
            do {
                if (wcscmp(find_file_data.cFileName, L".") != 0 && wcscmp(find_file_data.cFileName, L"..") != 0) {
                    if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        std::unique_lock<std::mutex> lock(queue_mtx);
                        directories_to_search.push(current_dir_str + L"\\" + find_file_data.cFileName);
                        tasks_in_progress++;
                        work_cv.notify_one();
                    } else {
                        if (wcsstr(find_file_data.cFileName, search_filename.c_str()) != nullptr) {
                            local_found_map[current_dir_str].push_back(find_file_data.cFileName);
                        }
                    }
                }
            } while (FindNextFileW(h_find, &find_file_data) != 0);
            FindClose(h_find);
        }
        tasks_in_progress--;
    }

    if (!local_found_map.empty()) {
        std::lock_guard<std::mutex> lock(all_thread_results_mtx);
        all_thread_results.push_back(std::move(local_found_map));
    }
}

int wmain(int argc, wchar_t* argv[]) {
    enable_virtual_terminal_processing();

    if (argc != 2) {
        std::wcerr << L"Usage: fsearch \"filename\"" << std::endl;
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    search_filename = argv[1];

    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        wchar_t logical_drives[MAX_PATH];
        GetLogicalDriveStringsW(MAX_PATH, logical_drives);
        wchar_t* drive = logical_drives;
        while (*drive) {
            directories_to_search.push(drive);
            tasks_in_progress++;
            drive += wcslen(drive) + 1;
        }
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(search_worker);
    }

    std::thread animation_thread(show_animation);

    while (tasks_in_progress > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        work_cv.notify_all();
    }

    done = true;
    work_cv.notify_all();

    for (auto& t : threads) {
        t.join();
    }

    searching_animation = false;
    animation_thread.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // --- Fusion et affichage des résultats ---
    std::map<std::wstring, std::vector<std::wstring>> final_grouped_results;
    size_t total_files = 0;
    for (auto& thread_map : all_thread_results) {
        for (auto& pair : thread_map) {
            total_files += pair.second.size();
            final_grouped_results[pair.first].insert(
                final_grouped_results[pair.first].end(),
                std::make_move_iterator(pair.second.begin()),
                std::make_move_iterator(pair.second.end())
            );
        }
    }

    std::wcout << L"\n--- Search Results ---\n" << std::endl;
    for (const auto& pair : final_grouped_results) {
        std::wcout << ANSI_COLOR_CYAN << pair.first << L"\\" << ANSI_COLOR_RESET << std::endl;
        for (const auto& filename : pair.second) {
            std::wcout << L"  -> " << ANSI_COLOR_YELLOW << filename << ANSI_COLOR_RESET << std::endl;
        }
        std::wcout << std::endl;
    }

    std::wcout << L"--- Summary ---\n";
    std::wcout << L"Found " << total_files << " file(s) in " << final_grouped_results.size() << " directorie(s)." << std::endl;
    std::wcout << L"Time elapsed: " << elapsed.count() << " seconds." << std::endl;

    return 0;
}
