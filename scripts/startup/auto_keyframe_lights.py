# SPDX-License-Identifier: GPL-2.0-or-later
# Auto-Keyframe Lights - Startup Script
# Automatically adds delta_scale keyframes to new lights to ensure proper material updates

import bpy
from bpy.app.handlers import persistent

print("[Auto-Keyframe Lights] Module loading...")

# Global set to track which lights we've already keyframed
_keyframed_lights = set()

@persistent
def check_for_new_lights(scene, depsgraph):
    """Check for new lights and keyframe them using depsgraph updates."""
    for update in depsgraph.updates:
        if update.is_updated_geometry:
            id_data = update.id
            if isinstance(id_data, bpy.types.Object) and id_data.type == 'LIGHT':
                obj = id_data
                obj_id = id(obj)
                
                if obj_id not in _keyframed_lights:
                    try:
                        # Ensure delta_scale has a value
                        if obj.delta_scale == (0.0, 0.0, 0.0):
                            obj.delta_scale = (1.0, 1.0, 1.0)
                        
                        # Add keyframe
                        obj.keyframe_insert(data_path="delta_scale", frame=scene.frame_current)
                        _keyframed_lights.add(obj_id)
                        
                        # Force viewport and UI refresh
                        for area in bpy.context.screen.areas:
                            if area.type == 'VIEW_3D':
                                area.tag_redraw()
                            elif area.type == 'DOPESHEET_EDITOR':
                                area.tag_redraw()
                            elif area.type == 'GRAPH_EDITOR':
                                area.tag_redraw()
                        
                        # Update the scene to ensure animation system is aware
                        scene.frame_set(scene.frame_current)
                        
                        print(f"[Auto-Keyframe] ✓ Keyframed delta_scale for light: {obj.name}")
                    except Exception as e:
                        print(f"[Auto-Keyframe] ✗ Failed to keyframe light {obj.name}: {e}")

@persistent
def clear_tracking_on_load(dummy):
    """Clear the tracking set when a new file is loaded."""
    global _keyframed_lights
    _keyframed_lights.clear()

# Register handlers
if check_for_new_lights not in bpy.app.handlers.depsgraph_update_post:
    bpy.app.handlers.depsgraph_update_post.append(check_for_new_lights)

if clear_tracking_on_load not in bpy.app.handlers.load_post:
    bpy.app.handlers.load_post.append(clear_tracking_on_load)

print("[Auto-Keyframe Lights] ✓ Handlers registered successfully")
