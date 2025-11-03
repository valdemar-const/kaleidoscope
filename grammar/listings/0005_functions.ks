# test function declaration/definition syntax

# minimal
function foo(a, b, c);
function log_point(a, b, c);

# full
function foo(a: i32, b: i8, c: f64): baz;

# full definition
function foo(a: i32, b: i8, c: f64): baz do
    log_point();
    return a + b + c;
end

function main() do
    return foo(a, b, c);
end
