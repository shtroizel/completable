#include "AbstractWindow.h"

#include <algorithm>

#include <ncurses.h>

#include "AbstractTab.h"
#include "CompletionStack.h"
#include "Settings.h"
#include "Layer.h"
#include "VisibilityAspect.h"
#include "key_codes.h"



// minimum required terminal height
static int const MIN_ROOT_Y{27};

// minimum required terminal width
static int const MIN_ROOT_X{80};



AbstractWindow::AbstractWindow()
    : tabs{std::make_shared<Tab::Flags>()}
    , disabled{std::make_shared<VisibilityAspect::Flags>()}
    , left_neighbor{std::make_shared<matchable::MatchBox<Tab::Type, AbstractWindow *>>()}
    , right_neighbor{std::make_shared<matchable::MatchBox<Tab::Type, AbstractWindow *>>()}
{
}


AbstractWindow::~AbstractWindow()
{
    delwin(w);
    w = nullptr;
}


void AbstractWindow::clear()
{
    if (nullptr != w)
    {
        wclear(w);
        wrefresh(w);
    }
}


void AbstractWindow::resize()
{
    if (!is_enabled())
        return;

    if (nullptr != w)
    {
        clear();
        delwin(w);
        w = nullptr;
    }

    getmaxyx(stdscr, root_y, root_x);
    resize_hook();
    w = newwin(height, width, y, x);
    post_resize_hook();
    mark_dirty();
}


void AbstractWindow::draw(bool clear_first)
{
    if (!is_enabled())
        return;

    // check terminal for minimum size requirement
    if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
    {
        wclear(w);
        wrefresh(w);
        return;
    }

    // always draw when clear_first regardless of dirty
    if (!dirty && !clear_first)
        return;

    if (clear_first)
        wclear(w);

    // border
    if (EnablednessSetting::Borders::grab().as_enabledness() == Enabledness::Enabled::grab() && borders_enabled())
        box(w, 0, 0);

    // title
    {
        int indent = width - (int) ((width / 1.618 + title().length() / 2.0) + 0.5);
        mvwaddch(w, 0, indent - 1, ' ');
        {
            int const active_indicator_left = is_active() ? '>' : ' ';
            int const active_indicator_right = is_active() ? '<' : ' ';
            mvwaddch(w, 0, indent - 2, active_indicator_left);
            mvwaddch(w, 0, title().size() + indent + 1, active_indicator_right);
        }
        for (int i = 0; i < (int) title().size(); ++i)
            mvwaddch(w, 0, i + indent, title()[i]);
        mvwaddch(w, 0, title().size() + indent, ' ');
    }

    // window specific drawing
    draw_hook();

    dirty = false;

    wrefresh(w);
}


Layer::Type AbstractWindow::get_layer() const
{
    return layer();
}


void AbstractWindow::set_left_neighbor(Tab::Type tab, AbstractWindow * n)
{
    if (nullptr == n)
        return;

    if (layer() != n->layer())
        return;

    left_neighbor->set(tab, n);
}


void AbstractWindow::set_right_neighbor(Tab::Type tab, AbstractWindow * n)
{
    if (nullptr == n)
        return;

    if (layer() != n->layer())
        return;

    right_neighbor->set(tab, n);
}


AbstractWindow * AbstractWindow::get_left_neighbor(Tab::Type tab)
{
    if (tab.is_nil())
        return nullptr;

    return left_neighbor->at(tab);
}


AbstractWindow * AbstractWindow::get_right_neighbor(Tab::Type tab)
{
    if (tab.is_nil())
        return nullptr;

    return right_neighbor->at(tab);
}


bool AbstractWindow::is_active()
{
    if (!belongs_to_active_tab())
        return false;

    auto act_win = AbstractTab::get_active_tab().as_AbstractTab()->get_active_window();
    if (nullptr == act_win)
        return false;

    return act_win == this;
}


bool AbstractWindow::belongs_to_active_tab()
{
    auto active_tab = AbstractTab::get_active_tab();
    if (active_tab.is_nil())
        return false;

    return tabs->is_set(active_tab);
}


void AbstractWindow::add_tab(AccessKey_AbstractWindow_add_tab, Tab::Type tab)
{
    tabs->set(tab);
}


void AbstractWindow::on_KEY(int key)
{
    if (!is_enabled())
        return;

    if (!is_active())
        return;

    switch (key)
    {
        case KEY_LEFT           : activate_left(AbstractTab::get_active_tab());  return;
        case KEY_RIGHT          : activate_right(AbstractTab::get_active_tab()); return;
        case KEY_UP             : on_KEY_UP();                                   return;
        case KEY_DOWN           : on_KEY_DOWN();                                 return;
        case PAGE_UP            : on_PAGE_UP();                                  return;
        case PAGE_DOWN          : on_PAGE_DOWN();                                return;
        case HOME               : on_HOME();                                     return;
        case END                : on_END();                                      return;
        case RETURN             : on_RETURN();                                   return;
        case DELETE             : on_DELETE();                                   return;
        case TAB                : on_TAB();                                      return;
        case KEY_BACKSPACE      :
        case BACKSPACE_127      :
        case BACKSPACE_BKSLSH_B : on_BACKSPACE();                                return;
    }

    if (key > 31 && key < 127) // printable ascii
        on_printable_ascii(key);
}


void AbstractWindow::mark_dirty()
{
    dirty = true;
    for (auto dep : dirty_dependencies)
        dep->mark_dirty();
}


void AbstractWindow::add_dirty_dependency(AbstractWindow * dep)
{
    dirty_dependencies.push_back(dep);
}


void AbstractWindow::set_enabled(bool enabledness, VisibilityAspect::Type aspect)
{
    if (enabledness)
        enable(aspect);
    else
        disable(aspect);
}


bool AbstractWindow::is_enabled() const
{
    return 0 == disabled->currently_set().size();
}


bool AbstractWindow::is_enabled(VisibilityAspect::Type aspect) const
{
    return !disabled->is_set(aspect);
}


void AbstractWindow::enable(VisibilityAspect::Type aspect)
{
    disabled->unset(aspect);
    if (is_enabled())
        resize();
}


void AbstractWindow::disable(VisibilityAspect::Type aspect)
{
    if (is_enabled())
    {
        pre_disable_hook();

        if (nullptr != w)
        {
            clear();
            delwin(w);
            w = nullptr;
        }
    }

    disabled->set(aspect);
    mark_dirty();
}


void AbstractWindow::activate_left(Tab::Type tab)
{
    activate_neighbor(tab, [&](AbstractWindow * win, Tab::Type t) { return win->get_left_neighbor(t); });
}


void AbstractWindow::activate_right(Tab::Type tab)
{
    activate_neighbor(tab, [&](AbstractWindow * win, Tab::Type t) { return win->get_right_neighbor(t); });
}


void AbstractWindow::activate_neighbor(
    Tab::Type tab,
    std::function<AbstractWindow * (AbstractWindow *, Tab::Type)> get_neighbor
)
{
    AbstractWindow * n = get_neighbor(this, tab);
    while (nullptr != n && n != this && !n->is_enabled(VisibilityAspect::WindowVisibility::grab()))
        n = get_neighbor(n, tab);

    if (nullptr != n && n != this && n->tabs->is_set(tab))
        tab.as_AbstractTab()->set_active_window(n);
}
