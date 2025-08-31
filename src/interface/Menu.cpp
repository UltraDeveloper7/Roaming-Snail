#include "../precompiled.h"
#include "Menu.hpp"

namespace {
    // One source of truth for the help lines
    static const char* kHelpLines[] = {
        "Edit options with LEFT/RIGHT",
        "Move camera: W S A D E Q",
        "Rotate camera: mouse",
        "Adjust power: UP/DOWN",
        "Rotate cue: LEFT/RIGHT",
        "Strike: SPACE",
        "Toggle lights: keys 0-9",
    };
    constexpr int kHelpCount = static_cast<int>(sizeof(kHelpLines) / sizeof(kHelpLines[0]));

    // Generic renderer for the help text; draws at any anchor/scale/alignment
    static void RenderHelpBlock(
        Menu& menu, float uAnchor, float vTop,
        Alignment align, float scale, float lineGap)
    {
        float v = vTop;
        for (int i = 0; i < kHelpCount; ++i) {
            menu.AddText(uAnchor, v, kHelpLines[i], scale, align, false);
            v -= lineGap;
        }
    }
}

// --------------------------------------------------------------------//

Menu::Menu(const int width, const int height) : width_(width), height_(height) {
    selected_ = -1;
    last_mouse_ = GLFW_RELEASE;
}

void Menu::Update(const int width, const int height) { width_ = width; height_ = height; }

void Menu::beginFrameCaptureMouse()
{
    texts_.clear();
    GLFWwindow* w = glfwGetCurrentContext();
    double mx, my;
    glfwGetCursorPos(w, &mx, &my);
    int ww, wh;
    glfwGetWindowSize(w, &ww, &wh);
    mouse_x_ = mx;
    mouse_y_ = wh - my;
}

float Menu::estimateWidthPx(const std::string& s, float scale) const
{
    constexpr float kAvg = 0.62f; // rough glyph width
    const float base = static_cast<float>(Config::default_font_size);
    return static_cast<float>(s.size()) * base * kAvg * scale;
}
float Menu::estimateHeightPx(float scale) const
{
    const float base = static_cast<float>(Config::default_font_size);
    return base * 1.1f * scale;
}

bool Menu::button(float u, float v, const std::string& label, float scale,
    Alignment align, bool emphasize, float* out_w, float* out_h)
{
    // compute rect
    float px = u * width_;
    float py = v * height_;
    float w = estimateWidthPx(label, scale);
    float h = estimateHeightPx(scale);

    float x0 = px, x1 = px + w;
    if (align == Alignment::CENTER) { x0 = px - w * 0.5f; x1 = px + w * 0.5f; }
    else if (align == Alignment::RIGHT) { x0 = px - w; x1 = px; }

    float y0 = py - h * 0.5f;
    float y1 = py + h * 0.5f;

    // hover?
    bool hover = (mouse_x_ >= x0 && mouse_x_ <= x1 && mouse_y_ >= y0 && mouse_y_ <= y1);

    // draw
    AddText(u, v, label, scale, align, emphasize || hover);

    // CLICK: frame-wide edge (set in Draw), requires hover
    bool clicked = hover && mouse_edge_down_;

    if (out_w) *out_w = (x1 - x0);
    if (out_h) *out_h = (y1 - y0);
    return clicked;
}

bool Menu::ConsumeEditedNames(std::string& outP1, std::string& outP2) {
    if (!names_dirty_) return false;
    outP1 = p1_name_;
    outP2 = p2_name_;
    names_dirty_ = false;
    return true;
}

bool Menu::ConsumePlayClicked() { bool b = play_clicked_;  play_clicked_ = false; return b; }
bool Menu::ConsumeExitClicked() { bool b = exit_clicked_;  exit_clicked_ = false; return b; }
bool Menu::ConsumeResetClicked() { bool b = reset_clicked_; reset_clicked_ = false; return b; }

void Menu::DrawMainMenu(bool modalOpen, int winW, int winH,
    float mouseX, float mouseY, int curEnter,
    int& prevEnter, int& selected)
{
    auto isEnterOn = [&](int logicalIndex)->bool {
        if (selected == -1) return false;
        const bool pressed = (curEnter == GLFW_PRESS && prevEnter == GLFW_RELEASE);
        return pressed && (selected == logicalIndex);
        };

    // Play
    const bool playSelected = (selected == 0);
    if (!modalOpen) {
        bool playClicked = button(0.5f, 0.70f, "Play", Ui(1.05f), Alignment::CENTER, playSelected, nullptr, nullptr) || isEnterOn(0);
        if (playClicked) play_clicked_ = true;
    }

    // Quick Setup (opens modal)
    if (!settings_open_) {
        const bool qsSelected = (selected == 1);
        const char* dd = settings_open_ ? "v" : ">";
        bool qsClicked = !modalOpen &&
            button(0.10f, 0.18f, std::string(dd) + "  Quick Setup", Ui(0.85f), Alignment::LEFT, qsSelected, nullptr, nullptr);
        if (qsClicked || isEnterOn(1)) { settings_open_ = true; selected = 1; }
    }

    // How to Play (opens modal)
    if (!help_open_) {
        const bool helpSel = (selected == 2);
        bool helpClicked = !modalOpen &&
            button(0.10f, 0.10f, "[i] How to Play", Ui(0.90f), Alignment::LEFT, helpSel, nullptr, nullptr);
        if (helpClicked || isEnterOn(2)) { help_open_ = true; selected = 2; }
    }
}

void Menu::DrawPauseMenu(bool modalOpen, int winW, int winH,
    float mouseX, float mouseY, int curEnter,
    int& prevEnter, int& selected)
{
    auto isEnterOn = [&](int i)->bool {
        if (selected == -1) return false;
        bool pressed = (curEnter == GLFW_PRESS && prevEnter == GLFW_RELEASE);
        return pressed && (selected == i);
        };

    if (!modalOpen) {
        const bool resumeSel = (selected == 0);
        bool resumeClicked = button(0.5f, 0.84f, "Resume game", Ui(1.05f), Alignment::CENTER, resumeSel, nullptr, nullptr) || isEnterOn(0);
        if (resumeClicked) play_clicked_ = true;

        const float startV = 0.64f, stepV = 0.10f;

        const bool resetSel = (selected == 1);
        if (button(0.5f, startV + 0 * stepV, "Reset game", Ui(0.90f), Alignment::CENTER, resetSel, nullptr, nullptr) || isEnterOn(1)) {
            reset_clicked_ = true;
            rename_gate_open_ = true;  // allow editing names this pause session
        }

        const bool qsSel = (selected == 2);
        const char* dd = settings_open_ ? "v" : ">";
        if (button(0.5f, startV - 1 * stepV, std::string(dd) + "  Quick Setup", Ui(0.90f), Alignment::CENTER, qsSel, nullptr, nullptr) || isEnterOn(2)) {
            settings_open_ = true; selected = 2;
        }

        const bool helpSel = (selected == 3);
        if (button(0.5f, startV - 2 * stepV, "[i] How to Play", Ui(0.90f), Alignment::CENTER, helpSel, nullptr, nullptr) || isEnterOn(3)) {
            help_open_ = true; selected = 3;
        }

        const bool exitSel = (selected == 4);
        if (button(0.5f, startV - 3 * stepV, "Exit game", Ui(0.90f), Alignment::CENTER, exitSel, nullptr, nullptr) || isEnterOn(4)) {
            exit_clicked_ = true;
        }
    }
}


void Menu::DrawQuickSetupModal(int winW, int winH, float mouseX, float mouseY, bool has_started)
{
    static int focus = 0; // 0 = strike force, 1 = friction

    const float panelU = 0.42f;
    const float headV = 0.60f;
    const float row1V = 0.54f;
    const float row2V = 0.49f;
    const float closeV = 0.43f;

    AddText(panelU, headV, "Quick Setup  v", Ui(0.9f), Alignment::LEFT, true);

    const float lineScale = Ui(0.8f);

    // Strike force (arrows only)
    std::string sf = std::format("Strike force: {:.2f}", Config::power_coeff);
    float sfWidthPx = estimateWidthPx(sf, lineScale);
    const float sfX0 = panelU * winW;
    const float rowH = estimateHeightPx(lineScale);
    const float row1Y0 = row1V * winH - rowH * 0.5f;
    const float row1Y1 = row1V * winH + rowH * 0.5f;

    const bool row1Hover = (mouseX >= sfX0 && mouseX <= sfX0 + sfWidthPx && mouseY >= row1Y0 && mouseY <= row1Y1);
    if (row1Hover) focus = 0;
    AddText(panelU, row1V, sf, lineScale, Alignment::LEFT, (focus == 0) || row1Hover);

    float leftU = (sfX0 + sfWidthPx + 18.0f) / winW;
    float rightU = (sfX0 + sfWidthPx + 48.0f) / winW;
    if (button(leftU, row1V, "<", lineScale, Alignment::CENTER, false, nullptr, nullptr))
        Config::power_coeff -= 0.05f;
    if (button(rightU, row1V, ">", lineScale, Alignment::CENTER, false, nullptr, nullptr))
        Config::power_coeff += 0.05f;

    // Ball friction (arrows only), fine step ±0.0005, range [0.9400..0.9995]
    std::string bf = std::format("Ball friction: {:.4f}", Config::linear_damping);
    float bfWidthPx = estimateWidthPx(bf, lineScale);
    const float bfX0 = panelU * winW;
    const float row2Y0 = row2V * winH - rowH * 0.5f;
    const float row2Y1 = row2V * winH + rowH * 0.5f;

    const bool row2Hover = (mouseX >= bfX0 && mouseX <= bfX0 + bfWidthPx && mouseY >= row2Y0 && mouseY <= row2Y1);
    if (row2Hover) focus = 1;
    AddText(panelU, row2V, bf, lineScale, Alignment::LEFT, (focus == 1) || row2Hover);

    leftU = (bfX0 + bfWidthPx + 18.0f) / winW;
    rightU = (bfX0 + bfWidthPx + 48.0f) / winW;

    if (button(leftU, row2V, "<", lineScale, Alignment::CENTER, false, nullptr, nullptr)) {
        Config::linear_damping = std::max(0.940f, Config::linear_damping - 0.0005f);
        Config::velocity_multiplier = Config::linear_damping;
    }
    if (button(rightU, row2V, ">", lineScale, Alignment::CENTER, false, nullptr, nullptr)) {
        Config::linear_damping = std::min(0.9995f, Config::linear_damping + 0.0005f);
        Config::velocity_multiplier = Config::linear_damping;
    }

    // Close
    if (button(panelU, closeV, "Close [Esc]", Ui(0.8f), Alignment::LEFT, false, nullptr, nullptr)) {
        settings_open_ = false;
        selected_ = has_started ? 2 : 1;
    }

    // keyboard in modal
    static int prevUp = GLFW_RELEASE, prevDown = GLFW_RELEASE, prevLeft = GLFW_RELEASE, prevRight = GLFW_RELEASE, prevEsc = GLFW_RELEASE;
    GLFWwindow* w = glfwGetCurrentContext();
    int kUp = glfwGetKey(w, GLFW_KEY_UP);
    int kDown = glfwGetKey(w, GLFW_KEY_DOWN);
    int kLeft = glfwGetKey(w, GLFW_KEY_LEFT);
    int kRight = glfwGetKey(w, GLFW_KEY_RIGHT);
    int kEsc = glfwGetKey(w, GLFW_KEY_ESCAPE);

    if (kUp == GLFW_PRESS && prevUp == GLFW_RELEASE)   focus = 0;
    if (kDown == GLFW_PRESS && prevDown == GLFW_RELEASE) focus = 1;

    if (kLeft == GLFW_PRESS && prevLeft == GLFW_RELEASE) {
        if (focus == 0) Config::power_coeff -= 0.05f;
        else {
            Config::linear_damping = std::max(0.940f, Config::linear_damping - 0.0005f);
            Config::velocity_multiplier = Config::linear_damping;
        }
    }
    if (kRight == GLFW_PRESS && prevRight == GLFW_RELEASE) {
        if (focus == 0) Config::power_coeff += 0.05f;
        else {
            Config::linear_damping = std::min(0.9995f, Config::linear_damping + 0.0005f);
            Config::velocity_multiplier = Config::linear_damping;
        }
    }
    if (kEsc == GLFW_PRESS && prevEsc == GLFW_RELEASE) {
        settings_open_ = false;
        selected_ = has_started ? 2 : 1;
    }

    prevUp = kUp; prevDown = kDown; prevLeft = kLeft; prevRight = kRight; prevEsc = kEsc;
}


void Menu::DrawHelpModal(bool has_started)
{
    AddText(0.5f, 0.62f, "[i] How to Play", Ui(1.1f), Alignment::CENTER, true);

    RenderHelpBlock(*this, /*u*/0.5f, /*vTop*/0.56f, Alignment::CENTER,
        Ui(0.75f), /*lineGap*/0.045f);

    if (button(0.5f, 0.56f - 7 * 0.045f - 0.05f, "Close [Esc]", Ui(0.8f), Alignment::CENTER, false, nullptr, nullptr)) {
        help_open_ = false;
        selected_ = has_started ? 3 : 2;
    }

    static int prevEscHelp = GLFW_RELEASE;
    GLFWwindow* w = glfwGetCurrentContext();
    int kEsc = glfwGetKey(w, GLFW_KEY_ESCAPE);
    if (kEsc == GLFW_PRESS && prevEscHelp == GLFW_RELEASE) {
        help_open_ = false;
        selected_ = has_started ? 3 : 2;
    }
    prevEscHelp = kEsc;
}

void Menu::DrawSettingsIcon(int /*winW*/, int /*winH*/)
{
    // Shown only when ui_settings_open_ == false (caller guards this)
    if (button(0.97f, 0.06f, "*", Ui(1.1f), Alignment::RIGHT, false, nullptr, nullptr)) {
        ui_settings_open_ = true;
        active_input_ = -1;
    }
}


void Menu::DrawUiSettingsModal(bool has_started)
{
    // More vertical breathing room (extra spacing everywhere)
    const float headV = 0.70f;          // title
    const float row1V = headV - 0.09f; // UI scale presets
    const float guideV = row1V - 0.09f;  // Aiming guide (more gap below the scale)
    const float row2V = guideV - 0.11f; // Player 1  (extra gap under aiming row)
    const float row3V = row2V - 0.09f;  // Player 2
    const float hintV = row3V - 0.08f;  // “Names are editable…” (shown during pause)
    const float closeV = hintV - 0.08f;  // Close [Esc] (extra gap below the hint)


    AddText(0.5f, headV, "Settings", Ui(1.1f), Alignment::CENTER, true);

    // --- UI Scale presets (50/75/100) laid out by measured width with fixed px gap ---
    struct Opt { const char* txt; float val; };
    static constexpr Opt kOpts[] = { {"50%",0.50f}, {"75%",0.75f}, {"100%",1.00f} };
    static constexpr int kOptCount = 3;

    const float cbScale = Ui(0.90f);
    const float gapPx = 36.0f * ui_scale_; // consistent physical gap
    std::string labels[kOptCount];
    float widths[kOptCount];
    float totalPx = 0.0f;

    for (int i = 0; i < kOptCount; ++i) {
        const bool on = std::fabs(ui_scale_ - kOpts[i].val) < 0.001f;
        labels[i] = std::string(on ? "[x] " : "[ ] ") + kOpts[i].txt;
        widths[i] = estimateWidthPx(labels[i], cbScale) * ui_scale_;
        totalPx += widths[i];
    }
    totalPx += gapPx * (kOptCount - 1);

    float x = 0.5f * width_ - 0.5f * totalPx;  // left edge so the whole row is centered
    for (int i = 0; i < kOptCount; ++i) {
        const bool on = std::fabs(ui_scale_ - kOpts[i].val) < 0.001f;
        const float centerU = (x + widths[i] * 0.5f) / static_cast<float>(width_);
        if (button(centerU, row1V, labels[i], cbScale, Alignment::CENTER, on, nullptr, nullptr)) {
            ui_scale_ = kOpts[i].val;
        }
        x += widths[i] + gapPx;
    }

    // --- Aiming guideline toggle ---
    const std::string gLabel = std::string(show_guideline_ ? "[x] " : "[ ] ") + "Aiming guide";
    if (button(0.5f, guideV, gLabel, Ui(0.90f), Alignment::CENTER, show_guideline_, nullptr, nullptr)) {
        show_guideline_ = !show_guideline_;
    }

    // --- Player names (gated by reset) ---
    const bool canEditNames = (!has_started) || rename_gate_open_;

    auto nameRow = [&](float v, const char* label, int fieldIndex, std::string& target)
        {
            std::string field = target;
            if (active_input_ == fieldIndex && canEditNames) {
                const bool caretOn = std::fmod(glfwGetTime(), 1.0) < 0.5;
                field += caretOn ? "|" : " ";
            }
            AddText(0.5f, v, std::string(label) + field, Ui(0.95f), Alignment::CENTER, active_input_ == fieldIndex);

            if (!canEditNames) return;

            const float labelW = estimateWidthPx(label, Ui(0.95f)) * ui_scale_;
            const float fieldW = std::max(240.0f * ui_scale_, estimateWidthPx("MMMMMMMMMMMMMMMM", Ui(0.95f)) * ui_scale_);
            const float pxMid = 0.5f * width_;
            const float x0 = pxMid - (labelW + fieldW) * 0.5f + labelW;
            const float x1 = x0 + fieldW;
            const float y = v * height_;
            const float h = estimateHeightPx(Ui(0.95f)) * ui_scale_;
            const float y0 = y - h * 0.5f, y1 = y + h * 0.5f;

            if (mouse_edge_down_ && mouse_x_ >= x0 && mouse_x_ <= x1 && mouse_y_ >= y0 && mouse_y_ <= y1) {
                active_input_ = fieldIndex;
            }
        };

    nameRow(row2V, "Player 1: ", 0, p1_name_);
    nameRow(row3V, "Player 2: ", 1, p2_name_);

    // --- Typing handler (ONLY when a field is active) ---
    if (canEditNames && (active_input_ == 0 || active_input_ == 1)) {
        GLFWwindow* w = glfwGetCurrentContext();
        auto& target = (active_input_ == 0 ? p1_name_ : p2_name_);
        const size_t maxLen = 18;

        auto keyEdge = [&](int key)->bool {
            static int prev[512] = { 0 };
            int cur = glfwGetKey(w, key);
            bool e = (cur == GLFW_PRESS && prev[key] == GLFW_RELEASE);
            prev[key] = cur; return e;
            };
        const bool shift =
            (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
            (glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

        if (keyEdge(GLFW_KEY_BACKSPACE)) { if (!target.empty()) { target.pop_back(); names_dirty_ = true; } }
        if (keyEdge(GLFW_KEY_ENTER) || keyEdge(GLFW_KEY_KP_ENTER)) active_input_ = -1;

        for (int k = GLFW_KEY_A; k <= GLFW_KEY_Z; ++k) if (keyEdge(k)) {
            char c = static_cast<char>('a' + (k - GLFW_KEY_A));
            if (shift) c = static_cast<char>(std::toupper(c));
            if (target.size() < maxLen) { target.push_back(c); names_dirty_ = true; }
        }
        for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; ++k) if (keyEdge(k)) {
            char c = static_cast<char>('0' + (k - GLFW_KEY_0));
            if (target.size() < maxLen) { target.push_back(c); names_dirty_ = true; }
        }
        if (keyEdge(GLFW_KEY_SPACE)) { if (target.size() < maxLen) { target.push_back(' '); names_dirty_ = true; } }
        if (keyEdge(GLFW_KEY_MINUS)) { if (target.size() < maxLen) { target.push_back(shift ? '_' : '-'); names_dirty_ = true; } }
    }

    // --- ALWAYS VISIBLE/ACTIVE below (even if no field is focused) ---
    if (has_started && !rename_gate_open_) {
        AddText(0.5f, hintV, "Names are editable after Reset game.", Ui(0.75f), Alignment::CENTER, false);
    }

    // Close button
    if (button(0.5f, closeV, "Close [Esc]", Ui(0.85f), Alignment::CENTER, false, nullptr, nullptr)) {
        ui_settings_open_ = false;
        active_input_ = -1;
        rename_gate_open_ = false; // one-time gate
    }

    // Esc to close
    static int prevEsc = GLFW_RELEASE;
    const int kEsc = glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE);
    if (kEsc == GLFW_PRESS && prevEsc == GLFW_RELEASE) {
        ui_settings_open_ = false;
        active_input_ = -1;
        rename_gate_open_ = false;
    }
    prevEsc = kEsc;
}


void Menu::Draw(const bool not_loaded, const bool has_started)
{
    texts_.clear();
    GLFWwindow* w = glfwGetCurrentContext();

    const int winW = width_, winH = height_;
    double mx, my; glfwGetCursorPos(w, &mx, &my);
    const float mouseX = static_cast<float>(mx);
    const float mouseY = static_cast<float>(winH - my);
    mouse_x_ = mouseX; mouse_y_ = mouseY;

    static int prevMouse = GLFW_RELEASE;
    const  int curMouse = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT);
    static int prevEnter = GLFW_RELEASE;
    const  int curEnter = glfwGetKey(w, GLFW_KEY_ENTER);

    mouse_edge_down_ = (curMouse == GLFW_PRESS && prevMouse == GLFW_RELEASE);

    const bool modalOpen = settings_open_ || help_open_ || ui_settings_open_;

    if (!modalOpen) ControlState();

    // Title
    AddText(0.5f, 0.90f, "8 Ball Pool", Ui(2.0f), Alignment::CENTER);

    // Main vs Pause
    if (!has_started) DrawMainMenu(modalOpen, winW, winH, mouseX, mouseY, curEnter, prevEnter, selected_);
    else              DrawPauseMenu(modalOpen, winW, winH, mouseX, mouseY, curEnter, prevEnter, selected_);

    // Modals
    if (settings_open_)    DrawQuickSetupModal(winW, winH, mouseX, mouseY, has_started);
    if (help_open_)        DrawHelpModal(has_started);
    if (ui_settings_open_) DrawUiSettingsModal(has_started);

    if (!ui_settings_open_) {
        DrawSettingsIcon(winW, winH);
    }

    prevMouse = curMouse;
    prevEnter = curEnter;
}

void Menu::ControlState()
{
    GLFWwindow* window = glfwGetCurrentContext();

    // DOWN: move selection down; if nothing selected yet, start at 0
    const int down_state = glfwGetKey(window, GLFW_KEY_DOWN);
    if (down_state == GLFW_PRESS && last_down_state_ != GLFW_PRESS) {
        if (selected_ == -1) selected_ = 0; else selected_++;
    }
    last_down_state_ = down_state;

    // UP: move selection up; if nothing selected yet, start at 0
    const int up_state = glfwGetKey(window, GLFW_KEY_UP);
    if (up_state == GLFW_PRESS && last_up_state_ != GLFW_PRESS) {
        if (selected_ == -1) selected_ = 0; else selected_--;
    }
    last_up_state_ = up_state;
}

void Menu::AddText(const float u, const float v, const std::string& text,
    const float scale, Alignment alignment, const bool selected)
{
    // multiply the per-item scale; TextRenderer already multiplies this into its font_scale_
    texts_.emplace_back(u * width_, v * height_, text, scale * ui_scale_, alignment, selected);
}
