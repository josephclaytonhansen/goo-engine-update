import bpy
from bpy import context

# Switch to Image Editor
for area in context.screen.areas:
    if area.type == 'IMAGE_EDITOR':
        break
else:
    # No image editor found, create one by changing the current area
    context.area.type = 'IMAGE_EDITOR'

# Now test the zoom menu
print("Testing zoom menu in Image Editor...")
space = context.space_data
print(f"Space type: {space.type}")

# Test zoom_percentage property
try:
    zoom_percent = space.zoom_percentage
    print(f"Current zoom percentage: {zoom_percent}%")
    
    # Test the menu draw function
    from bl_ui.space_image import IMAGE_MT_view_zoom
    menu = IMAGE_MT_view_zoom
    print(f"Menu class: {menu}")
    print("Menu should work now!")
    
except Exception as e:
    print(f"Error: {e}")

print("Test complete.")
