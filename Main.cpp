#include <stdint.h>
#include <stddef.h>
#include <time.h> 

// --- V18: CUSTOM BARE-METAL C-STRING AND I/O FUNCTIONS (Remain) ---
void memcpy(void* dest, const void* src, size_t n) { /* ... */ } // Required for COPY

// ... (VGADriver, SysCall, PIC, IDT, ResolutionEngine, app_list definitions remain) ...

// --- V18: FILE SYSTEM UTILITIES ---

FsNode* find_node_in_dir(FsNode* dir, const char* name) {
    if (!dir->is_directory) return nullptr;
    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (dir->children[i] != nullptr && strstr(dir->children[i]->name, name) != nullptr) {
            return dir->children[i];
        }
    }
    return nullptr;
}

// V18: Recursive DIR /S Logic
void recursive_dir_list(FsNode* dir, int depth, bool recurse) {
    for (int i = 0; i < MAX_CHILDREN; ++i) {
        FsNode* node = dir->children[i];
        if (node != nullptr) {
            // Print Indentation
            for(int j = 0; j < depth; ++j) SysCall::Print("  ");
            
            if (node->is_directory) {
                SysCall::Print(node->name, VGADriver::CYAN);
                SysCall::Print(" <DIR>\n");
                
                if (recurse) {
                    recursive_dir_list(node, depth + 1, recurse);
                }
            } else {
                SysCall::Print(node->name, VGADriver::WHITE);
                SysCall::Print("  ");
                SysCall::Print(itoa_custom(node->size), VGADriver::YELLOW);
                SysCall::Print(" bytes\n");
            }
        }
    }
}

FsNode* allocate_new_node() {
    for (int i = 0; i < 128; ++i) {
        // We use the node's name[0] as a simple 'used' flag
        if (node_pool[i].name[0] == '\0') {
            return &node_pool[i];
        }
    }
    return nullptr; // Disk Full
}

// --- V18: APP IMPLEMENTATIONS (Internal Commands) ---

void app_dir(const char* args) {
    if (current_directory == nullptr) return; // Safety check

    bool recurse = (strstr(args, "/S") != nullptr);
    SysCall::Print("Directory of ", VGADriver::CYAN);
    // In a real OS, you'd print the path here
    SysCall::Print("RAM:\\\n", VGADriver::CYAN); 
    
    recursive_dir_list(current_directory, 0, recurse);
}

void app_copy(const char* args) {
    // 1. Simple argument parsing (finds the space between src and dest)
    const char* src_name = args;
    const char* dest_name_ptr = strstr(args, " ");
    
    if (dest_name_ptr == nullptr) {
        SysCall::Print("Usage: COPY <source> <destination>\n", VGADriver::RED);
        return;
    }
    
    // Copy source name out (up to 12 chars)
    char src[MAX_FILENAME];
    for(int i = 0; i < MAX_FILENAME && src_name[i] != ' ' && src_name[i] != '\0'; ++i) src[i] = src_name[i];
    src[MAX_FILENAME - 1] = '\0';

    const char* dest_name = dest_name_ptr + 1;

    // 2. Find source node
    FsNode* src_node = find_node_in_dir(current_directory, src);
    if (src_node == nullptr || src_node->is_directory) {
        SysCall::Print("Source file not found or is a directory.\n", VGADriver::RED);
        return;
    }

    // 3. Allocate new destination node
    FsNode* dest_node = allocate_new_node();
    if (dest_node == nullptr) {
        SysCall::Print("Error: RAM Disk full.\n", VGADriver::RED);
        return;
    }
    
    // 4. Copy data and metadata
    memcpy(dest_node->data, src_node->data, src_node->size);
    strcpy(dest_node->name, dest_name);
    dest_node->size = src_node->size;
    dest_node->is_directory = false;
    dest_node->parent = current_directory;

    // 5. Link destination node to the current directory
    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (current_directory->children[i] == nullptr) {
            current_directory->children[i] = dest_node;
            break;
        }
    }

    SysCall::Print("File copied successfully.\n", VGADriver::GREEN);
}

void app_cd(const char* args) {
    if (strstr(args, "..") != nullptr) {
        // CD .. logic
        if (current_directory->parent != nullptr) {
            current_directory = current_directory->parent;
        }
    } else {
        // CD <dir> logic
        FsNode* target = find_node_in_dir(current_directory, args);
        if (target != nullptr && target->is_directory) {
            current_directory = target;
        } else {
            SysCall::Print("Directory not found.\n", VGADriver::RED);
        }
    }
    SysCall::Print("Current Dir: ", VGADriver::YELLOW);
    SysCall::Print(current_directory->name, VGADriver::YELLOW);
    SysCall::Print("\n", VGADriver::WHITE);
}

void app_budget(const char* args) {
    SysCall::Print("Budgeting Software V1.0 - Budget: $0.00 (Unimplemented)\n", VGADriver::GREEN);
}

// --- V18: Update Command Table ---
// V18: Update App List (Now includes new DOS commands)
Executable app_list[] = {
    {"CALC",     app_calc,     "Simple Calculator (Internal)"},
    {"FNAF",     app_fnaf,     "Security Protocol (Internal)"},
    {"DIR",      app_dir,      "List Files (Supports /S)"}, // Updated
    {"CLS",      app_cls,      "Clear Screen"},
    {"HELP",     app_help,     "Show this help menu"},
    {"GAME",     app_game,     "V9 Status and Resolution Engine (External)"},
    {"INVADERS", app_invaders, "Launch Space Invaders V1 (External)"},
    {"COPY",     app_copy,     "Copy Source to Destination"}, // New
    {"CD",       app_cd,       "Change Directory (CD .. supported)"}, // New
    {"BUDGET",   app_budget,   "Budgeting Utility"}, // New
    {"\0",       nullptr,      "\0"} 
};

// --- V18: KERNEL ENTRY POINT (Initialization) ---
extern "C" void kernel_main() {
    // ... (VGA clear, IDT setup remains) ...
    
    // V18: Initialize RAM Disk and Root Node
    FsNode* root = &node_pool[0];
    strcpy(root->name, "RAM_ROOT");
    root->is_directory = true;
    current_directory = root; // Start user in the root directory

    // V18: Create a dummy directory and file for DIR /S testing
    FsNode* sys_dir = allocate_new_node();
    strcpy(sys_dir->name, "SYSTEM");
    sys_dir->is_directory = true;
    sys_dir->parent = root;
    root->children[0] = sys_dir;

    FsNode* log_file = allocate_new_node();
    strcpy(log_file->name, "LOG.TXT");
    log_file->size = 128;
    log_file->is_directory = false;
    log_file->parent = sys_dir;
    sys_dir->children[0] = log_file;

    // ... (VGA print headers remains) ...

    screen.print("=========================================\n", VGADriver::CYAN);
    screen.print("    SD/OS Chaos Core V18.0 Boot\n", VGADriver::CYAN);
    screen.print("    Hierarchical File System Active\n", VGADriver::CYAN);
    screen.print("=========================================\n\n", VGADriver::CYAN);
    screen.print("[SYSTEM]: F/S Initialized. Current Dir: RAM_ROOT.\n", VGADriver::GREEN);
    
    asm volatile("sti"); 
    screen.print("\nSD/OS> ", VGADriver::WHITE);
    while(1) { __asm__ __volatile__ ("hlt"); }
}
