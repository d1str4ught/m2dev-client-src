import os
import shutil
import subprocess
from pathlib import Path
import time
import stat

# --- Configuration ---
# Define a central list of all dependencies
DEPENDENCIES = [
	# Full Submodules (Clone and leave intact)
	{
		"name": "cryptopp",
		"type": "extract",
		"repo": "https://github.com/weidai11/cryptopp",
		"target_dir": "vendor/cryptopp",
		"extract": [],
	},
	{
		"name": "mio",
		"type": "submodule",
		"repo": "https://github.com/vimpunk/mio",
		"target_dir": "vendor/mio",
	},
	{
		"name": "zstd",
		"type": "submodule",
		"repo": "https://github.com/facebook/zstd",
		"target_dir": "vendor/zstd",
	},
	# File/Folder Extraction (Temporary Clone & Cleanup)
	{
		"name": "lzo",
		"type": "extract",
		"repo": "https://github.com/synaptseal/lzo-2.10",
		"target_dir": "vendor/lzo-2.10",
		"extract": [("include", "."), ("src", ".")], # (source_in_repo, target_in_dest)
	},
	{
		"name": "DirectXMath",
		"type": "extract",
		"repo": "https://github.com/microsoft/DirectXMath",
		"target_dir": "vendor/DirectXMath",
		"extract": [("build", ".")], # (source_in_repo, target_in_dest)
	},
	{
		"name": "stb",
		"type": "extract",
		"repo": "https://github.com/nothings/stb",
		"target_dir": "extern/include",
		"extract": [("stb_image.h", "."), ("stb_image_write.h", ".")],
	},
	{
		"name": "pcg-cpp",
		"type": "extract",
		"repo": "https://github.com/imneme/pcg-cpp",
		"target_dir": "extern/include",
		"extract": [
			("include/pcg_random.hpp", "."),
			("include/pcg_extras.hpp", "."),
			("include/pcg_uint128.hpp", "."),
		],
	},
	{
		"name": "argparse",
		"type": "extract",
		"repo": "https://github.com/p-ranav/argparse",
		"target_dir": "extern/include",
		"extract": [("include/argparse/argparse.hpp", ".")]
	},
	{
		"name": "miniaudio",
		"type": "extract",
		"repo": "https://github.com/mackron/miniaudio",
		"target_dir": "extern/include",
		"extract": [("miniaudio.c", "."), ("miniaudio.h", ".")],
	},
	{
		"name": "rapidjson",
		"type": "extract",
		"repo": "https://github.com/Tencent/rapidjson",
		"target_dir": "extern/include",
		"extract": [("include/rapidjson", "rapidjson")],
	},
	{
		"name": "wil",
		"type": "extract",
		"repo": "https://github.com/microsoft/wil",
		"target_dir": "extern/include",
		"extract": [("include/wil", "wil")],
	},
]

# --- Utility Functions ---

def run_git_command(command, check_error=True):
	"""Executes a git command and handles errors."""
	try:
		# NOTE: We now use command as a list of arguments, not a single string.
		# This requires adjusting how it's called in handle_submodule.
		subprocess.run(
			command,
			check=check_error,
			# shell=True is removed for security/portability
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			text=True
		)
	except subprocess.CalledProcessError as e:
		# Convert list back to string for clean error reporting
		print(f"[ERROR] ERROR: Git command failed: {' '.join(command)}")
		print(f"Stderr: {e.stderr}")
		if check_error:
			raise

def handle_submodule(dep):
	"""Adds or updates a dependency as a standard Git submodule."""
	target_path = Path(dep["target_dir"])
	repo_url = dep["repo"]
	
	# 1. Check if the directory exists AND if Git recognizes it as a submodule path
	# We use a cleaner check: If the directory exists AND it's tracked in .gitmodules
	is_submodule_tracked = False
	if Path(".gitmodules").exists():
		# Check if the target directory path is in the .gitmodules file
		gitmodules_content = Path(".gitmodules").read_text()
		if dep['target_dir'] in gitmodules_content:
			is_submodule_tracked = True
			
	if is_submodule_tracked:
		print(f"[UPDATE] Updating submodule: {dep['name']}...")
		# Now pass the command as a list of strings
		command = ["git", "submodule", "update", "--remote", "--", dep['target_dir']]
		run_git_command(command)
		return False # No new submodule was added
	
	# 2. Add the submodule (only runs if not tracked in .gitmodules)
	print(f"[NEW] Adding submodule: {dep['name']}...")
	# Add command as a list
	command = ["git", "submodule", "add", "--force", repo_url, dep['target_dir']]
	run_git_command(command)
	return True # New submodule was added

def handle_extraction(dep):
	"""Downloads a dependency, extracts files/folders, and cleans up."""
	name = dep["name"]
	repo_url = dep["repo"]
	target_path = Path(dep["target_dir"])
	tmp_path = Path(f".tmp_{name}") # Use a unique temp directory

	print(f"[UPDATE] Managing extraction dependency: {name}...")

	if tmp_path.exists():
		print(f"   Pre-cleanup: Deleting stale temporary directory: {tmp_path}")
		try:
			shutil.rmtree(tmp_path, onerror=handle_remove_readonly)
			print("   Cleanup successful.")
		except Exception as e:
			print(f"   [ERROR] Final cleanup failed with error: {e}")
			raise

	# 1. Clean up existing files in the target directory that will be replaced
	# This ensures a clean update, crucial for file-extraction deps.
	for src_file, dest_file in dep['extract']:
		target_item = target_path / (dest_file if dest_file != "." else Path(src_file).name)
		if target_item.is_file():
			print(f"   Deleting old file: {target_item}")
			target_item.unlink()
		elif target_item.is_dir():
			print(f"   Deleting old folder: {target_item}")
			try:
				shutil.rmtree(target_item, onerror=handle_remove_readonly)
			except Exception as e:
				print(f"   ERROR during target cleanup for {target_item}: {e}")
				raise # Propagate the error to halt the script
	
	# 2. Clone into a temporary directory
	print(f"   Cloning into temporary directory: {tmp_path}")
	# Using --depth 1 to make the temporary clone fast and shallow
	clone_command = ["git", "clone", "--depth", "1", repo_url, str(tmp_path)]
	run_git_command(clone_command)

	# 3. Create the target directory if it doesn't exist
	target_path.mkdir(parents=True, exist_ok=True)

	# 4. Move/Copy files/folders
	# --- 4a. SPECIAL HANDLING FOR CryptoPP ---
	if name == "cryptopp":
		tmp_dir = tmp_path
		target_root = target_path # vendor/cryptopp
		cmakelists_file = target_root / "CMakeLists.txt"
		backup_path = Path(".tmp_cmakelists_cryptopp")

		# 1. BACKUP: Move your custom CMakeLists.txt to a safe location
		if cmakelists_file.exists():
			print("   Backing up custom CMakeLists.txt...")
			try:
				shutil.move(cmakelists_file, backup_path)
			except Exception as e:
				print(f"   ERROR during CMakeLists.txt backup: {e}")
				raise

		# 2. AGGRESSIVE CLEANUP: Delete the entire vendor/cryptopp directory
		print(f"    Aggressively deleting old target directory: {target_root}")
		if target_root.exists():
			try:
				# Delete the whole folder with aggressive error handling
				shutil.rmtree(target_root, onerror=handle_remove_readonly)
				print("    Old target cleanup successful.")
			except Exception as e:
				print(f"   ERROR during target cleanup for {target_root}: {e}")
				raise
		
		# 3. RESTORE & COPY: Recreate the directory and copy content
		target_root.mkdir(parents=True, exist_ok=True)
		
		# Copy ALL contents from temp repo root into vendor/cryptopp
		print(f"    Copying ALL source files to {target_root}")
		for item in tmp_dir.iterdir():
			# Skip hidden .git directory and any repo-specific CMake files
			if item.name.startswith('.'):
				continue
			
			destination = target_root / item.name
			
			if item.is_dir():
				shutil.copytree(item, destination, dirs_exist_ok=True)
			elif item.is_file():
				shutil.copy2(item, destination)

		# 4. RESTORE: Move the custom CMakeLists.txt back
		if backup_path.exists():
			print("     restoring custom CMakeLists.txt...")
			shutil.move(backup_path, cmakelists_file)

	# --- 4b. SPECIAL HANDLING FOR DirectXMath (Add 'elif' here) ---
	elif name == "DirectXMath":
		# Source: The 'build' subdirectory inside the temporary clone
		source_dir = tmp_path / "build"
		# Destination: The target directory (vendor/DirectXMath)
		destination_dir = target_path
		
		print(f"   Copying CONTENTS of DirectXMath/build to {destination_dir}")
		
		if source_dir.exists():
			# Copy all files from source_dir into destination_dir
			for item in source_dir.iterdir():
				if item.is_file():
					shutil.copy2(item, destination_dir / item.name)
			
			# NOTE: If DirectXMath had subfolders that needed copying, 
			# you'd need to add shutil.copytree logic here as well. 
			# Assuming files only, based on typical header libraries.
		else:
			print(f"   [ERROR] DirectXMath 'build' folder not found at {source_dir}. Aborting.")
			# We raise an error here since the primary component is missing
			raise FileNotFoundError(f"DirectXMath 'build' folder missing.")

	# --- 4b. DEFAULT EXTRACTION HANDLING (For all other dependencies) ---
	else:
		for src_in_repo, dest_in_target in dep["extract"]:
			source = tmp_path / src_in_repo
			
			# Determine the final destination path
			if dest_in_target == ".":
				# If destination is '.', use the source's name as the final name
				destination = target_path / source.name
			else:
				destination = target_path / dest_in_target
				
			# Move or copy the item
			if source.exists():
				print(f"   Copying {source.name} to {destination}")
				if source.is_dir():
					# For folders (like rapidjson, wil, lzo)
					shutil.copytree(source, destination, dirs_exist_ok=True)
				else:
					# For single files (like stb, pcg-cpp)
					shutil.copy2(source, destination)
			else:
				 print(f"   [WARNING] Source path not found in temporary repo: {source}")

	# 5. Clean up the temporary directory
	print(f"   Cleaning up temporary directory: {tmp_path}")
	try:
		# Use the onerror handler for robust deletion on Windows
		shutil.rmtree(tmp_path, onerror=handle_remove_readonly)
		print("   Cleanup successful.")
	except Exception as e:
		print(f"   [ERROR] Final cleanup failed with error: {e}")
		raise # Re-raise the error if final cleanup fails
		
	return False

def final_git_update():
	"""Runs the final git submodule sync and update commands."""
	print("\n--- Finalizing Git Submodules ---")
	print("Running 'git submodule sync'")
	run_git_command("git submodule sync")
	print("Running 'git submodule update --init --recursive'")
	run_git_command("git submodule update --init --recursive")
	print("[DONE] Dependency installation complete!")

# --- Main Execution ---

def handle_remove_readonly(func, path, exc_info):
	"""
	Error handler for shutil.rmtree on Windows.
	If the file is readonly, change its permission and retry.
	"""
	# Check if the error is Access Denied (often happens with Git files)
	if func in (os.rmdir, os.remove, os.unlink) and exc_info[1].winerror == 5:
		# Change file permissions to writable
		os.chmod(path, stat.S_IWUSR | stat.S_IREAD)
		func(path)
	else:
		# Raise the exception if it's not a permission issue
		raise

def main():
	"""Main function to iterate through and manage all dependencies."""
	new_submodules_added = False
	
	for dep in DEPENDENCIES:
		print(f"\n--- Managing Dependency: {dep['name']} ---")
		
		try:
			if dep["type"] == "submodule":
				added = handle_submodule(dep)
				if added:
					new_submodules_added = True
			elif dep["type"] == "extract":
				handle_extraction(dep)
			else:
				print(f"⚠️ Unknown dependency type for {dep['name']}: {dep['type']}")
				
		except Exception as e:
			print(f"\nFATAL ERROR processing {dep['name']}. Aborting script.")
			print(e)
			return 1

	if new_submodules_added:
		final_git_update()
	else:
		print("\nAll dependencies were already present or updated. No new submodules added, skipping final sync/update.")
	
	return 0

if __name__ == "__main__":
	exit(main())