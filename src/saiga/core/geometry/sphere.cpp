/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "sphere.h"

#include "internal/noGraphicsAPI.h"
namespace Saiga
{
int Sphere::intersectAabb(const AABB& other) const
{
    if (!intersectAabb2(other)) return 0;

    for (int i = 0; i < 8; i++)
    {
        if (!contains(other.cornerPoint(i))) return 1;
    }



    return 2;
}

void Sphere::getMinimumAabb(AABB& box) const
{
    vec3 rad(r + 1, r + 1, r + 1);
    box.min = pos - rad;
    box.max = pos + rad;
}

bool Sphere::intersectAabb2(const AABB& other) const
{
    float s, d = 0;

    // find the square of the distance
    // from the sphere to the box
    for (long i = 0; i < 3; i++)
    {
        if (pos[i] < other.min[i])
        {
            s = pos[i] - other.min[i];
            d += s * s;
        }

        else if (pos[i] > other.max[i])
        {
            s = pos[i] - other.max[i];
            d += s * s;
        }
    }
    return d <= r * r;
}

bool Sphere::contains(vec3 p) const
{
    return length(vec3(p - pos)) < r;
}

bool Sphere::intersect(const Sphere& other) const
{
    return distance(other.pos, pos) < r + other.r;
}

float Sphere::sdf(vec3 p) const
{
    return (p - pos).norm() - r;
}

vec2 Sphere::projectedIntervall(const vec3& d) const
{
    vec2 ret;
    float t = dot(d, pos);
    ret[0]  = std::min(t - r, t + r);
    ret[1]  = std::max(t + r, t - r);
    return ret;
}

std::ostream& operator<<(std::ostream& os, const Saiga::Sphere& s)
{
    os << "Sphere: " << s.pos << s.r;
    return os;
}

}  // namespace Saiga
