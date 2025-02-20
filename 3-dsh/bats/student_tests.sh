#!/usr/bin/env bats

@test "Test builtin cd command" {
    TEST_DIR="test_dir_$(date +%s)"
    mkdir "$TEST_DIR"
    
    run ./dsh <<EOF
cd $TEST_DIR
pwd
exit
EOF

    rmdir "$TEST_DIR"
    [ "$status" -eq 0 ]
    [[ "$output" == *"$TEST_DIR"* ]]
}

@test "Test external command execution" {
    run ./dsh <<EOF
echo "hello world"
exit
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"hello world"* ]]
}

@test "Test quoted arguments" {
    run ./dsh <<EOF
echo "   preserved  spaces   "
exit
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"   preserved  spaces   "* ]]
}

@test "Test command not found" {
    run ./dsh <<EOF
nonexistent_command
exit
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"command not found"* ]]
}

@test "Test rc command (extra credit)" {
    run ./dsh <<EOF
invalid_command
rc
exit
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"127"* ]] || [[ "$output" == *"2"* ]]  # Depends on system's ENOENT value
}
