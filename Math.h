class Math {
    public:
        static float PI;
        static int sign(float num) {
            if (num < 0) return -1;
            if (num > 0) return 1;
            return 0;
        }
};
float Math::PI = 3.141592653f;
