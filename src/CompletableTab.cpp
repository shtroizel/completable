#include "CompletableTab.h"

#include "AbstractWindow.h"



CompletableTab::CompletableTab() : AbstractTab()
{
    Tab::completable::grab().set_AbstractTab(this);
}
