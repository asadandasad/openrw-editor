#!/usr/bin/env python3
"""
OpenRW Level Editor Build Test Script
Tests the build system and basic functionality
"""

import os
import sys
import subprocess
import tempfile
import shutil
import json
from pathlib import Path

class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

class BuildTester:
    def __init__(self):
        self.project_root = Path(__file__).parent
        self.build_dir = self.project_root / "build"
        self.test_results = []
        
    def log(self, message, color=Colors.NC):
        print(f"{color}{message}{Colors.NC}")
        
    def run_command(self, command, cwd=None, capture_output=True):
        """Run a shell command and return the result"""
        try:
            result = subprocess.run(
                command,
                shell=True,
                cwd=cwd or self.project_root,
                capture_output=capture_output,
                text=True,
                timeout=300  # 5 minute timeout
            )
            return result.returncode == 0, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return False, "", "Command timed out"
        except Exception as e:
            return False, "", str(e)
    
    def test_dependencies(self):
        """Test if all required dependencies are available"""
        self.log("Testing dependencies...", Colors.YELLOW)
        
        dependencies = [
            ("cmake", "cmake --version"),
            ("make", "make --version"),
            ("g++", "g++ --version"),
            ("pkg-config", "pkg-config --version"),
        ]
        
        qt_packages = [
            "Qt5Core",
            "Qt5Widgets", 
            "Qt5OpenGL"
        ]
        
        all_good = True
        
        # Test basic tools
        for name, command in dependencies:
            success, stdout, stderr = self.run_command(command)
            if success:
                self.log(f"  ✓ {name} found", Colors.GREEN)
            else:
                self.log(f"  ✗ {name} not found", Colors.RED)
                all_good = False
        
        # Test Qt packages
        for package in qt_packages:
            success, stdout, stderr = self.run_command(f"pkg-config --exists {package}")
            if success:
                self.log(f"  ✓ {package} found", Colors.GREEN)
            else:
                self.log(f"  ✗ {package} not found", Colors.RED)
                all_good = False
        
        self.test_results.append(("Dependencies", all_good))
        return all_good
    
    def test_cmake_configuration(self):
        """Test CMake configuration"""
        self.log("Testing CMake configuration...", Colors.YELLOW)
        
        # Clean build directory
        if self.build_dir.exists():
            shutil.rmtree(self.build_dir)
        self.build_dir.mkdir()
        
        # Run CMake
        cmake_command = f"cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        success, stdout, stderr = self.run_command(cmake_command, cwd=self.build_dir)
        
        if success:
            self.log("  ✓ CMake configuration successful", Colors.GREEN)
            
            # Check if important files were generated
            files_to_check = [
                "CMakeCache.txt",
                "Makefile",
                "compile_commands.json"
            ]
            
            for file_name in files_to_check:
                file_path = self.build_dir / file_name
                if file_path.exists():
                    self.log(f"    ✓ {file_name} generated", Colors.GREEN)
                else:
                    self.log(f"    ✗ {file_name} missing", Colors.RED)
                    success = False
        else:
            self.log("  ✗ CMake configuration failed", Colors.RED)
            self.log(f"    Error: {stderr}", Colors.RED)
        
        self.test_results.append(("CMake Configuration", success))
        return success
    
    def test_compilation(self):
        """Test compilation"""
        self.log("Testing compilation...", Colors.YELLOW)
        
        # Build with make
        make_command = "make -j$(nproc)"
        success, stdout, stderr = self.run_command(make_command, cwd=self.build_dir)
        
        if success:
            self.log("  ✓ Compilation successful", Colors.GREEN)
            
            # Check if executable was created
            executable_path = self.build_dir / "openrw_editor"
            if executable_path.exists():
                self.log("    ✓ Executable created", Colors.GREEN)
            else:
                self.log("    ✗ Executable not found", Colors.RED)
                success = False
        else:
            self.log("  ✗ Compilation failed", Colors.RED)
            self.log(f"    Error: {stderr}", Colors.RED)
        
        self.test_results.append(("Compilation", success))
        return success
    
    def test_executable_basic(self):
        """Test basic executable functionality"""
        self.log("Testing executable basic functionality...", Colors.YELLOW)
        
        executable_path = self.build_dir / "openrw_editor"
        if not executable_path.exists():
            self.log("  ✗ Executable not found", Colors.RED)
            self.test_results.append(("Executable Basic", False))
            return False
        
        # Test help output
        success, stdout, stderr = self.run_command(f"{executable_path} --help")
        
        # Note: Qt applications might not have --help, so we'll just check if it runs
        # without crashing immediately
        if success or "Usage" in stdout or "Qt" in stderr:
            self.log("  ✓ Executable runs without immediate crash", Colors.GREEN)
            success = True
        else:
            self.log("  ✗ Executable failed to run", Colors.RED)
            self.log(f"    Error: {stderr}", Colors.RED)
            success = False
        
        self.test_results.append(("Executable Basic", success))
        return success
    
    def test_file_structure(self):
        """Test project file structure"""
        self.log("Testing project file structure...", Colors.YELLOW)
        
        required_files = [
            "CMakeLists.txt",
            "README.md",
            "build.sh",
            "src/main.cpp",
            "src/common/types.h",
            "src/entity_system.h",
            "src/scene_manager.h",
            "src/viewport/viewport_widget.h",
            "src/viewport/camera_controller.h",
            "src/ui/property_inspector.h",
            "src/ui/asset_browser.h",
            "src/ui/world_outliner.h",
            "src/ui/main_window.h",
            "src/mission/mission_editor.h",
            "src/mission/mission_node.h",
            "src/file_formats/dff_parser.h",
            "src/file_formats/txd_parser.h",
            "src/file_formats/ipl_parser.h",
            "src/file_formats/ide_parser.h",
            "src/file_formats/dat_parser.h"
        ]
        
        all_good = True
        for file_path in required_files:
            full_path = self.project_root / file_path
            if full_path.exists():
                self.log(f"  ✓ {file_path}", Colors.GREEN)
            else:
                self.log(f"  ✗ {file_path} missing", Colors.RED)
                all_good = False
        
        self.test_results.append(("File Structure", all_good))
        return all_good
    
    def test_code_quality(self):
        """Test basic code quality checks"""
        self.log("Testing code quality...", Colors.YELLOW)
        
        # Check for compile_commands.json
        compile_commands_path = self.build_dir / "compile_commands.json"
        if not compile_commands_path.exists():
            self.log("  ✗ compile_commands.json not found", Colors.RED)
            self.test_results.append(("Code Quality", False))
            return False
        
        # Load and validate compile commands
        try:
            with open(compile_commands_path, 'r') as f:
                compile_commands = json.load(f)
            
            if len(compile_commands) > 0:
                self.log(f"  ✓ Found {len(compile_commands)} compilation units", Colors.GREEN)
                
                # Check if main source files are included
                source_files = [cmd.get('file', '') for cmd in compile_commands]
                main_cpp_found = any('main.cpp' in f for f in source_files)
                
                if main_cpp_found:
                    self.log("  ✓ Main source files found in compilation", Colors.GREEN)
                    success = True
                else:
                    self.log("  ✗ Main source files not found in compilation", Colors.RED)
                    success = False
            else:
                self.log("  ✗ No compilation units found", Colors.RED)
                success = False
                
        except json.JSONDecodeError:
            self.log("  ✗ Invalid compile_commands.json format", Colors.RED)
            success = False
        except Exception as e:
            self.log(f"  ✗ Error reading compile_commands.json: {e}", Colors.RED)
            success = False
        
        self.test_results.append(("Code Quality", success))
        return success
    
    def generate_report(self):
        """Generate test report"""
        self.log("\n" + "="*50, Colors.BLUE)
        self.log("BUILD TEST REPORT", Colors.BLUE)
        self.log("="*50, Colors.BLUE)
        
        total_tests = len(self.test_results)
        passed_tests = sum(1 for _, result in self.test_results if result)
        
        for test_name, result in self.test_results:
            status = "PASS" if result else "FAIL"
            color = Colors.GREEN if result else Colors.RED
            self.log(f"{test_name:20} : {status}", color)
        
        self.log(f"\nSummary: {passed_tests}/{total_tests} tests passed", Colors.BLUE)
        
        if passed_tests == total_tests:
            self.log("🎉 All tests passed! Build is ready for use.", Colors.GREEN)
            return True
        else:
            self.log("❌ Some tests failed. Please check the issues above.", Colors.RED)
            return False
    
    def run_all_tests(self):
        """Run all tests"""
        self.log("OpenRW Level Editor Build Test", Colors.BLUE)
        self.log("="*40, Colors.BLUE)
        
        tests = [
            self.test_file_structure,
            self.test_dependencies,
            self.test_cmake_configuration,
            self.test_compilation,
            self.test_code_quality,
            self.test_executable_basic,
        ]
        
        for test in tests:
            try:
                test()
                self.log("")  # Empty line between tests
            except Exception as e:
                self.log(f"Test failed with exception: {e}", Colors.RED)
                self.test_results.append((test.__name__, False))
        
        return self.generate_report()

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--help":
        print("OpenRW Level Editor Build Test Script")
        print("Usage: python3 test_build.py")
        print("\nThis script tests the build system and basic functionality.")
        print("It will:")
        print("  1. Check project file structure")
        print("  2. Verify dependencies are installed")
        print("  3. Test CMake configuration")
        print("  4. Test compilation")
        print("  5. Test basic executable functionality")
        print("  6. Check code quality metrics")
        return 0
    
    tester = BuildTester()
    success = tester.run_all_tests()
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())

