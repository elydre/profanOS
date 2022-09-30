class Voiture {
    int vitesse = 0;
public:
    void afficher(int addr);
    void accelerer(int acc);
};

int main(int addr, int arg) {

    Voiture v;
    v.afficher(addr);
    v.accelerer(80);
    v.afficher(addr);
    v.accelerer(-50);
    v.afficher(addr);

    return arg;
}

void Voiture::afficher(int addr) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*fskprint)(char * format, ...) = (void (*)(char*, ...)) get_func(38);

    fskprint((char*)"vitesse: %d\n", vitesse);    
}

void Voiture::accelerer(int acc) {
    vitesse += acc;
}
