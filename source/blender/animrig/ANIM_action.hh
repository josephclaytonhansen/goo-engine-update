/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup animrig
 *
 * \brief Functions to work with Actions.
 */

#include "RNA_types.hh"

#include "BLI_span.hh"

struct FCurve;
struct bAction;

namespace blender::animrig {
class Action;
}

namespace blender::animrig {
/**
 * Get (or add relevant data to be able to do so) F-Curve from the given Action,
 * for the given Animation Data block. This assumes that all the destinations are valid.
 * \param ptr: can be a null pointer.
 */
FCurve *action_fcurve_ensure(Main *bmain,
                             bAction *act,
                             const char group[],
                             PointerRNA *ptr,
                             const char rna_path[],
                             int array_index);

/**
 * Find the F-Curve from the given Action. This assumes that all the destinations are valid.
 */
FCurve *action_fcurve_find(bAction *act, const char rna_path[], int array_index);

/**
 * Deselect the keys of all actions in the Span. Duplicate entries are only visited once.
 */
void deselect_keys_actions(blender::Span<bAction *> actions);

/**
 * Deselect all keys within the action.
 */
void action_deselect_keys(bAction &action);

}  // namespace blender::animrig
