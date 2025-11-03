function foo(a: i32, b: i8, c: f64): baz;

function foo(a: i32, b: i8, c: f64) do
    log_point();
    return a + b + c;
end

function main(): void do
    foo(a, b, c);
end
