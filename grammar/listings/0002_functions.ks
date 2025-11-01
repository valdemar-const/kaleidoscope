function foo(a: i32, b: i8, c: f64): baz;

function foo(a: i32, b: i8, c: f64): baz do
    log_point();
    return a + b + c;
end;

function main(): void do
    hello();
end
