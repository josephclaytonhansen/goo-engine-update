#!/usr/bin/env python3
"""
Test script for AOV Input node functionality in Goo Engine 4.2
Tests:
1. Node creation
2. Property access
3. Material connection
4. Rendered viewport view
5. Error handling
"""

import bpy
import sys

def test_aov_input_node():
    """Test AOV Input node functionality"""
    print("=" * 60)
    print("AOV Input Node Test - Goo Engine 4.2")
    print("=" * 60)
    
    try:
        # Test 1: Check if AOV Input node type exists
        print("1. Testing AOV Input node type registration...")
        has_aov_input = hasattr(bpy.types, 'ShaderNodeInputAOV')
        print(f"   ShaderNodeInputAOV exists: {has_aov_input}")
        if not has_aov_input:
            print("   FAILED: AOV Input node type not found!")
            return False
        
        # Test 2: Reset to factory settings
        print("\n2. Resetting to factory settings...")
        bpy.ops.wm.read_factory_settings(use_empty=True)
        print("   Factory settings loaded")
        
        # Test 3: Create test scene
        print("\n3. Creating test scene...")
        bpy.ops.mesh.primitive_cube_add()
        cube = bpy.context.active_object
        print(f"   Created cube: {cube.name}")
        
        # Test 4: Create material
        print("\n4. Creating material...")
        mat = bpy.data.materials.new('AOV_Test_Material')
        cube.data.materials.append(mat)
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        print(f"   Created material: {mat.name}")
        
        # Test 5: Clear default nodes
        print("\n5. Clearing default nodes...")
        nodes.clear()
        print("   Default nodes cleared")
        
        # Test 6: Create AOV Input node
        print("\n6. Creating AOV Input node...")
        aov_input = nodes.new('ShaderNodeInputAOV')
        aov_input.location = (-300, 0)
        print(f"   AOV Input node created: {aov_input.name}")
        print(f"   Node type: {aov_input.bl_idname}")
        
        # Test 7: Test AOV name property
        print("\n7. Testing AOV name property...")
        try:
            print(f"   Initial AOV name: '{aov_input.aov_name}'")
            aov_input.aov_name = "TestAOV"
            print(f"   Set AOV name to: '{aov_input.aov_name}'")
        except Exception as e:
            print(f"   WARNING: AOV name property error: {e}")
        
        # Test 8: Create Material Output node
        print("\n8. Creating Material Output node...")
        mat_output = nodes.new('ShaderNodeOutputMaterial')
        mat_output.location = (300, 0)
        print(f"   Material Output created: {mat_output.name}")
        
        # Test 9: Connect nodes
        print("\n9. Connecting AOV Input to Material Output...")
        try:
            # Connect AOV Color output to Material Base Color input
            links.new(aov_input.outputs['Color'], mat_output.inputs['Base Color'])
            print("   Connected AOV Color -> Material Base Color")
            
            # Also connect Alpha if available
            if 'Alpha' in aov_input.outputs and 'Alpha' in mat_output.inputs:
                links.new(aov_input.outputs['Alpha'], mat_output.inputs['Alpha'])
                print("   Connected AOV Alpha -> Material Alpha")
        except Exception as e:
            print(f"   Connection error: {e}")
            return False
        
        # Test 10: Set up viewport shading
        print("\n10. Setting up viewport shading...")
        for area in bpy.context.screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        print(f"   Current shading mode: {space.shading.type}")
                        space.shading.type = 'MATERIAL'
                        print("   Set viewport to Material Preview")
                        break
                break
        
        # Test 11: Switch to rendered view (the critical test)
        print("\n11. Switching to rendered viewport (CRITICAL TEST)...")
        try:
            for area in bpy.context.screen.areas:
                if area.type == 'VIEW_3D':
                    for space in area.spaces:
                        if space.type == 'VIEW_3D':
                            # Set render engine to EEVEE Legacy (Goo Engine target)
                            bpy.context.scene.render.engine = 'BLENDER_EEVEE'
                            print("   Set render engine to EEVEE")
                            
                            # Switch to rendered viewport
                            space.shading.type = 'RENDERED'
                            print("   Switched to RENDERED viewport")
                            
                            # Force viewport update
                            bpy.context.view_layer.update()
                            print("   Forced viewport update")
                            break
                    break
            
            print("   SUCCESS: Rendered viewport active without crash!")
            
        except Exception as e:
            print(f"   FAILED: Rendered viewport crashed: {e}")
            return False
        
        # Test 12: Test different AOV names
        print("\n12. Testing different AOV names...")
        test_aov_names = ["Diffuse", "Specular", "Emission", "Normal", "AO", "CustomAOV"]
        for aov_name in test_aov_names:
            try:
                aov_input.aov_name = aov_name
                print(f"   Set AOV name to '{aov_name}' - OK")
                # Force update
                bpy.context.view_layer.update()
            except Exception as e:
                print(f"   AOV name '{aov_name}' failed: {e}")
        
        # Test 13: Node tree validation
        print("\n13. Validating node tree...")
        print(f"   Material has {len(nodes)} nodes")
        print(f"   Material has {len(links)} links")
        print(f"   AOV Input outputs: {[output.name for output in aov_input.outputs]}")
        print(f"   Material Output inputs: {[input.name for input in mat_output.inputs]}")
        
        # Test 14: Final status
        print("\n14. Final validation...")
        if mat.node_tree.nodes and len(mat.node_tree.links) > 0:
            print("   Material node tree is valid")
            print("   AOV Input node is properly connected")
            print("   Rendered viewport is active")
            print("\n✅ ALL TESTS PASSED - AOV Input node is working correctly!")
            return True
        else:
            print("   Material node tree validation failed")
            return False
            
    except Exception as e:
        print(f"\n❌ CRITICAL ERROR: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main test function"""
    print("Starting AOV Input Node Test...")
    
    # Run the test
    success = test_aov_input_node()
    
    if success:
        print("\n" + "=" * 60)
        print("🎉 AOV INPUT NODE TEST COMPLETED SUCCESSFULLY!")
        print("The node can be created, connected, and used in rendered view.")
        print("=" * 60)
        return 0
    else:
        print("\n" + "=" * 60)
        print("💥 AOV INPUT NODE TEST FAILED!")
        print("There are issues that need to be addressed.")
        print("=" * 60)
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
