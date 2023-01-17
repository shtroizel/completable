#pragma once

#include <functional>

#include <matchable/matchable.h>

#include "exec_long_task_with_busy_animation.h"



MATCHABLE(Enabledness, Enabled, Disabled);
PROPERTYx1_MATCHABLE(
    // property
    Enabledness::Type, enabledness,

    // matchable name
    EnablednessSetting,

    // variants
    Borders,
    CompletionList,
    Length_spc_Completion,
    Ordinal_spc_Summation,
    Antonyms
);

using animation_content = std::array<std::vector<std::string>, 24> const *;
PROPERTYx1_MATCHABLE(
    animation_content, content,

    Animation,

    esc_Default,
    esc_Cheers_spc_To_spc_107
)
PROPERTYx1_MATCHABLE(
    Animation::Type, animation,

    AnimationSetting,

    Busy_spc_Animation
)


MATCHABLE_VARIANT_PROPERTY_VALUE(EnablednessSetting, Borders, enabledness, Enabledness::Enabled::grab());
MATCHABLE_VARIANT_PROPERTY_VALUE(EnablednessSetting, CompletionList, enabledness, Enabledness::Enabled::grab());
MATCHABLE_VARIANT_PROPERTY_VALUE(EnablednessSetting, Length_spc_Completion, enabledness, Enabledness::Disabled::grab());
MATCHABLE_VARIANT_PROPERTY_VALUE(EnablednessSetting, Ordinal_spc_Summation, enabledness, Enabledness::Disabled::grab());
MATCHABLE_VARIANT_PROPERTY_VALUE(EnablednessSetting, Antonyms, enabledness, Enabledness::Disabled::grab());

MATCHABLE_VARIANT_PROPERTY_VALUE(AnimationSetting, Busy_spc_Animation, animation, Animation::esc_Default::grab());
