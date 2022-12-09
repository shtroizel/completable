#pragma once

#include <matchable/matchable.h>



MATCHABLE(LibraryState, Loaded, Unloaded, Linked)

PROPERTYx1_MATCHABLE(LibraryState::Type, state, MatchmakerState, Instance)

#ifdef MM_DYNAMIC_LOADING
MATCHABLE_VARIANT_PROPERTY_VALUE(MatchmakerState, Instance, state, LibraryState::Unloaded::grab())
#else
MATCHABLE_VARIANT_PROPERTY_VALUE(MatchmakerState, Instance, state, LibraryState::Linked::grab())
#endif
