#!/usr/bin/env python3
"""
Blender 4.2 Compositor Testing Script
Tests the GPU acceleration infrastructure and compositor nodes
"""

import bpy

def test_compositor():
    print('=== BLENDER 4.2 COMPOSITOR TESTING ===')
    
    # Reset to factory settings
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    
    print('\n1. Testing GPU Compositor Infrastructure:')
    
    # Test compositor device and precision properties
    has_device = hasattr(scene.render, 'compositor_device')
    has_precision = hasattr(scene.render, 'compositor_precision')
    
    print(f'  ✅ Scene compositor_device property: {has_device}')
    print(f'  ✅ Scene compositor_precision property: {has_precision}')
    
    if has_device:
        print(f'     Default compositor_device: {scene.render.compositor_device}')
    if has_precision:
        print(f'     Default compositor_precision: {scene.render.compositor_precision}')
    
    print('\n2. Testing Compositor Node System:')
    
    # Enable compositing
    scene.use_nodes = True
    if scene.node_tree:
        scene.node_tree.nodes.clear()
        print('  ✅ Compositor node system enabled')
    
    print('\n3. Testing Specific Compositor Node Types:')
    
    # Test individual node types
    test_nodes = [
        ('CompositorNodeSceneTime', 'Scene Time Node'),
        ('CompositorNodeDenoise', 'Denoise Node'),
        ('CompositorNodeOutputFile', 'File Output Node'),
        ('CompositorNodeVecBlur', 'Vector Blur Node'),
        ('CompositorNodeDilateErode', 'Dilate/Erode Node'),
        ('CompositorNodeSwitchView', 'Switch View Node'),
        ('CompositorNodeAlphaOver', 'Alpha Over Node'),
        ('CompositorNodeMixRGB', 'Mix RGB Node'),
        ('CompositorNodeCryptomatteV2', 'Cryptomatte V2 Node')
    ]
    
    working_nodes = []
    missing_nodes = []
    
    for node_type, node_name in test_nodes:
        try:
            if hasattr(bpy.types, node_type):
                node = scene.node_tree.nodes.new(node_type)
                working_nodes.append(node_name)
                print(f'  ✅ {node_name}: Available')
                
                # Test specific properties for key nodes
                if node_type == 'CompositorNodeSceneTime':
                    outputs = [output.name for output in node.outputs]
                    print(f'     Outputs: {outputs}')
                elif node_type == 'CompositorNodeDenoise':
                    if hasattr(node, 'prefilter'):
                        print(f'     Prefilter options available: {hasattr(node, "prefilter")}')
                
                # Clean up
                scene.node_tree.nodes.remove(node)
            else:
                missing_nodes.append(node_name)
                print(f'  ❌ {node_name}: Not available')
        except Exception as e:
            missing_nodes.append(node_name)
            print(f'  ❌ {node_name}: Error - {str(e)}')
    
    print('\n4. Testing Realtime Compositor Shaders:')
    
    # Check if realtime compositor shader files exist
    import os
    shader_path = os.path.join(bpy.utils.resource_path('LOCAL'), '..', '..', 'source', 'blender', 'compositor', 'realtime_compositor', 'shaders')
    print(f'  Shader path exists: {os.path.exists(shader_path)}')
    
    print('\n=== COMPOSITOR TESTING SUMMARY ===')
    print(f'Working nodes: {len(working_nodes)}/{len(test_nodes)}')
    print(f'GPU infrastructure: {"✅" if has_device and has_precision else "❌"}')
    print(f'Node system: {"✅" if scene.use_nodes else "❌"}')
    
    if working_nodes:
        print(f'✅ Working: {", ".join(working_nodes)}')
    if missing_nodes:
        print(f'❌ Missing: {", ".join(missing_nodes)}')
    
    print('\n=== COMPOSITOR TESTING COMPLETE ===')

if __name__ == '__main__':
    test_compositor()
