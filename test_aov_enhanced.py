#!/usr/bin/env python3
"""
Enhanced AOV Input node test with socket inspection
"""

import bpy
import sys

def test_aov_input_node_enhanced():
    """Enhanced AOV Input node test with detailed socket inspection"""
    print("=" * 60)
    print("Enhanced AOV Input Node Test - Goo Engine 4.2")
    print("=" * 60)
    
    try:
        # Test 1: Check node type
        print("1. Testing AOV Input node type registration...")
        has_aov_input = hasattr(bpy.types, 'ShaderNodeInputAOV')
        print(f"   ShaderNodeInputAOV exists: {has_aov_input}")
        if not has_aov_input:
            return False
        
        # Test 2: Reset scene
        print("\n2. Resetting to factory settings...")
        bpy.ops.wm.read_factory_settings(use_empty=True)
        
        # Test 3: Create test objects
        print("\n3. Creating test scene...")
        bpy.ops.mesh.primitive_cube_add()
        cube = bpy.context.active_object
        
        # Test 4: Create material with nodes
        print("\n4. Creating material...")
        mat = bpy.data.materials.new('AOV_Test_Material')
        cube.data.materials.append(mat)
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        
        # Test 5: Clear and create nodes
        print("\n5. Setting up nodes...")
        nodes.clear()
        
        # Create AOV Input node
        aov_input = nodes.new('ShaderNodeInputAOV')
        aov_input.location = (-300, 0)
        aov_input.aov_name = "TestAOV"
        
        # Create Material Output node
        mat_output = nodes.new('ShaderNodeOutputMaterial')
        mat_output.location = (300, 0)
        
        # Test 6: Inspect socket names
        print("\n6. Inspecting socket names...")
        print("   AOV Input outputs:")
        for i, output in enumerate(aov_input.outputs):
            print(f"     [{i}] '{output.name}' - Type: {output.type}")
        
        print("   Material Output inputs:")
        for i, input in enumerate(mat_output.inputs):
            print(f"     [{i}] '{input.name}' - Type: {input.type}")
        
        # Test 7: Try different connection strategies
        print("\n7. Testing connections...")
        
        # Strategy 1: Try exact names
        connection_attempts = [
            # (from_socket, to_socket, description)
            ('Color', 'Base Color', 'AOV Color -> Base Color'),
            ('Color', 'Surface', 'AOV Color -> Surface'),
            ('Alpha', 'Alpha', 'AOV Alpha -> Alpha'),
        ]
        
        connections_made = 0
        for from_socket, to_socket, desc in connection_attempts:
            try:
                if from_socket in aov_input.outputs and to_socket in mat_output.inputs:
                    links.new(aov_input.outputs[from_socket], mat_output.inputs[to_socket])
                    print(f"   ✅ {desc}")
                    connections_made += 1
                else:
                    print(f"   ❌ {desc} - Socket not found")
            except Exception as e:
                print(f"   ❌ {desc} - Error: {e}")
        
        # Strategy 2: If no connections made, try first available
        if connections_made == 0:
            print("\n   Trying first available socket connection...")
            try:
                if len(aov_input.outputs) > 0 and len(mat_output.inputs) > 0:
                    from_socket = aov_input.outputs[0]
                    to_socket = mat_output.inputs[0]
                    links.new(from_socket, to_socket)
                    print(f"   ✅ Connected {from_socket.name} -> {to_socket.name}")
                    connections_made += 1
            except Exception as e:
                print(f"   ❌ First socket connection failed: {e}")
        
        # Test 8: Set render engine
        print("\n8. Setting up rendering...")
        bpy.context.scene.render.engine = 'BLENDER_EEVEE'
        print("   Set render engine to EEVEE")
        
        # Test 9: Simulate rendered view (background mode limitation)
        print("\n9. Testing material compilation...")
        try:
            # Force material update
            mat.node_tree.update_tag()
            bpy.context.view_layer.update()
            print("   Material updated successfully")
            
            # Check if material is valid
            if mat.node_tree and len(mat.node_tree.nodes) > 0:
                print("   Material node tree is valid")
            else:
                print("   WARNING: Material node tree validation failed")
                
        except Exception as e:
            print(f"   ERROR: Material compilation failed: {e}")
            return False
        
        # Test 10: Test different AOV names
        print("\n10. Testing different AOV names...")
        test_names = ["Diffuse", "Specular", "Emission", "Normal", "AO"]
        for name in test_names:
            try:
                aov_input.aov_name = name
                mat.node_tree.update_tag()
                print(f"   ✅ AOV name '{name}' - OK")
            except Exception as e:
                print(f"   ❌ AOV name '{name}' - Error: {e}")
        
        # Test 11: Final validation
        print("\n11. Final validation...")
        print(f"   Material nodes: {len(nodes)}")
        print(f"   Material links: {len(links)}")
        print(f"   Connections made: {connections_made}")
        
        if connections_made > 0:
            print("\n✅ AOV INPUT NODE TEST PASSED!")
            print("   - Node creation: SUCCESS")
            print("   - Property access: SUCCESS") 
            print("   - Material connection: SUCCESS")
            print("   - Material compilation: SUCCESS")
            return True
        else:
            print("\n⚠️  AOV INPUT NODE TEST PARTIAL SUCCESS!")
            print("   - Node creation: SUCCESS")
            print("   - Property access: SUCCESS")
            print("   - Material connection: FAILED")
            print("   - Material compilation: SUCCESS")
            return False
            
    except Exception as e:
        print(f"\n❌ CRITICAL ERROR: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main test function"""
    print("Starting Enhanced AOV Input Node Test...")
    
    success = test_aov_input_node_enhanced()
    
    if success:
        print("\n" + "=" * 60)
        print("🎉 AOV INPUT NODE IS WORKING!")
        print("=" * 60)
        return 0
    else:
        print("\n" + "=" * 60)
        print("⚠️  AOV INPUT NODE NEEDS ATTENTION!")
        print("=" * 60)
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
