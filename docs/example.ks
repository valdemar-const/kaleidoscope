# learn kaleidoscope in y minutes
# это однострочный комментарий
#[
  а это многострочный!
  #[можно вкладывать]#
]#

module preambule {.link="ksrt".} # переопределен экспорт в файл библиотеки (so/dll, a/lib), по умолчанию: #{module_name}-#{major}.#{minor}.#{patch}.{dll,so,lib,a
  doc
    @brief Базовый модуль среды выполнения kaleidoscope

    Подключается самым первым средой выполнения перед исполнением абсолютно любого исходного кода.
    Задача модуля определение базовых типов данных и часто используемых операторов.
    В преамбуле определяется "ядро" языка.
  spec
    version = "1.0.0" # важно, сохранит от проблем совместимости.
  export
    # эта секция не обязательна, экспортируемые символы можно пометить ключевым слово pub в блоке реализации
    # более того эту секцию можно было бы актуализировать тулингом, синхронизируя pub реализации из implement

    # базовые типы
    type i8     {.primitive, internal.}; # аннотация 'primitive' означает "Это базовый, тривиально копируемый скалярный тип"
    type i16    {.primitive, internal.}; # аннотация 'internal' означает "Это должно быть реализовано средой выполнения"
    type i32    {.primitive, internal.};
    type i64    {.primitive, internal.};

    type u8     {.primitive, internal.};
    type u16    {.primitive, internal.};
    type u32    {.primitive, internal.};
    type u64    {.primitive, internal.};

    type f32    {.primitive, internal.};
    type f64    {.primitive, internal.};

    type bool   {.primitive, internal.}; # truly/falsy значение для логической арифметики

    type string {.internal.};            # встроенный строковый тип кодировки utf-8
    type char   {.primitive, internal.}; # unicode символ

    type byte = u8;
    type size = u64;

    # методы расширения типа. можно применить к имени типа, или любому значению данного типа. У данных методов низший приоритет

    function i8.sizeof()    : u64  {.internal.}; # пример встроенной функции для типа, возвращает размер 'memory layout' типа в байтах
    function i8.alignof()   : u64  {.internal.}; # пример встроенной функции для типа, возвращает размер 'memory layout' типа в байтах
    function i8.low()       : i8   {.internal.}; # пример встроенной функции для типа, возвращает минимальное допустимое значение
    function i8.high()      : i8   {.internal.}; # пример встроенной функции для типа, возвращает максимальное допустимое значение

    function i16.sizeof()   : u64  {.internal.};
    function i16.alignof()  : u64  {.internal.};
    function i16.low()      : i16  {.internal.};
    function i16.high()     : i16  {.internal.};

    function i32.sizeof()   : u64  {.internal.};
    function i32.alignof()  : u64  {.internal.};
    function i32.low()      : i32  {.internal.};
    function i32.high()     : i32  {.internal.};

    function i64.sizeof()   : u64  {.internal.};
    function i64.alignof()  : u64  {.internal.};
    function i64.low()      : i64  {.internal.};
    function i64.high()     : i64  {.internal.};

    function u8.sizeof()    : u64  {.internal.};
    function u8.alignof()   : u64  {.internal.};
    function u8.low()       : i8   {.internal.};
    function u8.high()      : i8   {.internal.};

    function u16.sizeof()   : u64  {.internal.};
    function u16.alignof()  : u64  {.internal.};
    function u16.low()      : i16  {.internal.};
    function u16.high()     : i16  {.internal.};

    function u32.sizeof()   : u64  {.internal.};
    function u32.alignof()  : u64  {.internal.};
    function u32.low()      : i32  {.internal.};
    function u32.high()     : i32  {.internal.};

    function u64.sizeof()   : u64  {.internal.};
    function u64.alignof()  : u64  {.internal.};
    function u64.low()      : i64  {.internal.};
    function u64.high()     : i64  {.internal.};

    function f32.sizeof()   : u64  {.internal.};
    function f32.alignof()  : u64  {.internal.};
    function f32.low()      : f32  {.internal.};
    function f32.high()     : f32  {.internal.};

    function f64.sizeof()   : u64  {.internal.};
    function f64.alignof()  : u64  {.internal.};
    function f64.low()      : f64  {.internal.};
    function f64.high()     : f64  {.internal.};

    function bool.sizeof()  : u64  {.internal.};
    function bool.alignof() : u64  {.internal.};
    function bool.low()     : bool {.internal.};
    function bool.high()    : bool {.internal.};

  implement # блок реализации модуля

    pub literal email(value: string): email = email("value") # literal suffix
    pub literal i64(value: i32): i64 = value.i64;
    pub literal i64(value: string): result[i64, string] = value.to_str();

    pub interface IFormatter
        function fmt(self, args: range[weak string]): string;
    end

    pub implement IFormatter for i32
        function fmt(self, args: range[weak string]): string do
            ... преобразование числа в строку ...
            var result = ""

        end
    end

    # В языке зарезервировано несколько операторов:
    #
    # | Op | Example                                                        |
    # | -  | -                                                              |
    # | () | a(b, c), apply (b, c) to a                                     |
    # | [] | a[b][c], sugar or array memory layout access                   |
    # | .  | a.b, sugar or structural type field access                     |
    # | ?  | a? returns bool for truly falsy types                          |
    # | ?. | a?.b, sugar or reference/optional semantic safe field access   |
    # | !  | a! unsafe dereference value/pointer semantic value             |
    # | !. | a!.b, sugar or reference/optional semantic unsafe field access |
    # | :: | a::b, explicit resolve symbol operator                         |
    # | ?: | value or operator for truly/optional data objects              |
    # | |> | a |> b(c) |> d, pipeline operator for call chain d(b(a(), c))  |

    # остальные на усмотрение пользователя,
    # хотя preambule определяет базовые математические, логические и битовые операторы,
    # но это не обязательно. Ядро языка позволяет тонко настраивать допустимые операторы для заданных типов.

    pub operator +(lhs: i32, rhs: i32): i32 {.internal, infix, priority=20, associativity=left.}; # оператор это всегда обычная функция

    pub operator ++(value: i32): ref i32 {.internal, prefix.};
    pub operator ++(value: i32): ref i32 {.internal, postfix.};

    # etc. for every type

    pub type cint = i32; # синоним типа
end # module preambule

from preambule import *; # Теперь в текущем чанке доступны экспортируемые преамбулой символы.

type email {.distinct.} = string; # distinct type is same as 'string' technically but not semantically. Compiler think it's both different

type AnyPrimitive   = any_of[bool, i64, f64, string];
type AnyFormattable = any_with[IFormatter] {.clone, copy, soo=16.};

function foo(a: cint, b: cint): cint; # предварительное объявление функции 'foo is callable<(cint, cint): cint>'

function main(args: range[string]): void
  doc
    @brief example func that do something
  spec
    effects: None
  do
    # встроенные в грамматику литералы
    true;  # логический литерал
    false; # логический литерал
    1;     # литерал числа => i32(1)
    "литерал строки"; # => string("литерал строки")

    # Использование встроенного variant типа.
    let any_p: AnyPrimitive = true; # sizeof(bool)

    # match variant type with automatic unwrap
    match (any_p)
      of bool as b   => { #[ use_bool(b);  ]#         }
      of i64  as int => { #[ use_int(int); ]#         }
      _              => { #[ do nothing by default ]# }
    end

    # using literals suffixes

    let wider          = 1'i64;
    let wider_from_str = "1"'i64.value_or(0);
    let my_email       = "my@mail.ru"'mail; # использование пользовательского литерала

    var x1: сint = 5;                               # объявление переменной x1 с инициализацией значения
    let x2: сint = 6;                               # объявление неизменяемой переменной x2 с инициализацией значения
    var x3: сint in range[cint.min..cint.max] = 5;  # объявление переменной x3 с инициализацией значения
    var y: сint in range[10..100]             = 10; # объявление переменной x принимающей значения от 10 до 100 включительно
    var z: сint in range[10..<100]            = 10; # объявление переменной x принимающей значения от 10 до 100 НЕ включительно
    var name: string = "имячко";                    # строковая переменная
    var format: string = "x=#{x}"

    x1 = 2 * (y - z) + foo(x2, x3) - name.len();    # любое матеметическое выражение с числами, переменными и вызовами функций.

    if expr then
        ...
    elif
        ...
    else
        ...
    end

    var x = if expr a else b;                 # if expression
    var y = match (expr) of a => b of _ => c; # if expression

    for arg in args do
      # ... use(arg)
    end
end
