def f(x) : return x*(x+1)/2 * 750
print(f(2))
print(f(3))

def f2(z):
    # on cherche x tel que x^2+x-(2*z / 750) = 0
    a = 1
    b = 1
    c = -(2*z / 750)
    delta = (b**2-4*a*c)
    solution1 = (-b+(delta)**0.5)/(2*a)
    solution2 = (-b-(delta)**0.5)/(2*a)
    print(f"solutions : {solution1} et {solution2}")
    print(f"vérification : {solution1*(solution1+1)/2 * 750} et {solution2*(solution2+1)/2 * 750}")
    print("Solution qui nous interesse : {solution1}")

f2(4500)