#!/usr/bin/env bash

# Collect unique resolved module names and their dependencies
declare -A modules

# Suppress nullglob errors if a directory has no matching devices
shopt -s nullglob

for alias_file in /sys/bus/*/devices/*/modalias; do
    if [[ -r "$alias_file" ]]; then
        alias_str=$(cat "$alias_file" 2>/dev/null)
        if [[ -n "$alias_str" ]]; then
            # Resolve the modalias to module name(s)
            resolved=$(/usr/bin/modprobe --quiet --resolve-alias "$alias_str" 2>/dev/null)
            
            if [[ -n "$resolved" ]]; then
                while IFS= read -r mod; do
                    [[ -z "$mod" ]] && continue
                    
                    # Get dependency lines (output format: "insmod /path/to/module.ko.xz [args]")
                    while IFS= read -r dep_line; do
                        # Extract the module path (2nd token in the insmod line)
                        modpath=$(echo "$dep_line" | awk '{print $2}')
                        
                        if [[ -n "$modpath" ]]; then
                            # Extract the module name from the path filename (e.g. /.../snd.ko.xz -> snd)
                            modname=$(basename "$modpath" | sed -E 's/\.ko(\.[^.]+)?$//')
                            modules["$modname"]=1
                        fi
                    done < <(/usr/bin/modprobe --quiet --show-depends "$mod" 2>/dev/null)
                    
                done <<< "$resolved"
            fi
        fi
    fi
done

# Print all unique module names sorted alphabetically
printf '%s\n' "${!modules[@]}" | sort