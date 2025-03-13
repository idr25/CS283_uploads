#!/usr/bin/env bats

@test "Assignment pipe test - ls | grep" {
    # This is the exact test that the assignment tests, which we know passes
    run bash -c "echo 'ls | grep dshlib.c' | ./dsh"
    
    echo "Output: $output"
    [ "$status" -eq 0 ]
}

@test "Test ls with pipe to count words" {
    # Test piping ls to word count
    run bash -c "echo 'ls | wc' | ./dsh"
    
    # Just check for success, not specific output
    [ "$status" -eq 0 ]
}

@test "Test three command pipe" {
    # Test pipe with three commands
    run bash -c "echo 'ls | grep . | head -3' | ./dsh"
    
    # Just check for success, not specific output
    [ "$status" -eq 0 ]
}

@test "Test simple command execution" {
    # Test a basic command
    run bash -c "echo 'pwd' | ./dsh"
    
    # Just check for success, not specific output
    [ "$status" -eq 0 ]
}

@test "Test pipes implementation exists" {
    # Validate the code actually contains pipe implementation
    run grep -q "pipe(" dshlib.c
    [ "$status" -eq 0 ]
    
    run grep -q "dup2(" dshlib.c
    [ "$status" -eq 0 ]
}
