## Reflection, test 3 is still in development. Dont use it for anything important

[Syntax](#syntax) \
[Stuffs that doesnt work](#stuffs-that-doesnt-work) \
[Stuffs to be added](#stuffs-to-be-added)

# Syntax

## Variables
```
-- Basic types are number, string, bool

let a: number = 10      -- All vars must be initialized
let b: string = 'Hello'
```

## Functions
```
-- This syntax doesnt care about function orders
-- This syntax can only be use in global scope

fn f(a: number) -> void
    print(a)
end


-- This syntax does care about function orders
-- Can be used in any scope

let f2: fn = fn(a: number) -> void
    print(a)
end
```

## Array
```
let arr: number[] = []
let arr2: number[] = [10, 20, 30]
```

## Struct
```
type Vec2 {
    x: number = 0,   -- Must be initialized
    y: number = 0,
}

let vec: Vec2 = {}   -- Structs are initialized like this


-- Struct contain other structs

type Rect {
    pos: Vec2 = {},
    sz: Vec2 = {},

    area: fn = fn() -> number    -- Member functions
        return self.sz.x * self.sz.y
    end
}


-- Inheritance

type RectWithNum from Rect {
    n: number = 10
}
```

# Stuffs that doesnt work

These will get fixed in future versions

Array index doesnt have type checking, and you cant put a function
as array index

```
fn get2() -> number
	return 2
end

let a: number[] = [10, 20, 30, 40]

-- x doesnt exist. But this wont get type checked
print(a[x])

-- This wont work
print(a[get2()])
```

You also cant do something like this
```
type Vec2 {
	x: number = 0,
	y: number = 0
}

fn aaa() -> Vec2
	let v: Vec2 = {}
	v.x = 2
	v.y = 3
	return v
end

-- This wont work
print(aaa().x)
```

# Stuffs to be added
Tuple (multi return) \
continue statement

I may add more to this list later
