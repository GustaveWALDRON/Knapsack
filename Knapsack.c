#include <stdio.h>
#include <stdlib.h>

//objet
struct objet {
    int poids;
    int valeur;
    struct objet * suivant;
};
typedef struct objet * ListeObjets;

//arbre
struct noeud{
    struct objet * lobjet;
    int nbObjet;
    struct noeud * fils;
    struct noeud * frere;
};
typedef struct noeud * Arbre;

//pile de noeud pour les noeud à continuer à explorer
struct element {
    struct noeud * noeud;
    int poids;
    int valeur;
    struct element * suivant;
};
typedef struct element * PileNoeud;



//empiler dans la pile
void empiler(PileNoeud * p, Arbre * a, int poi, int val) {
    PileNoeud el = malloc(sizeof(struct element));
    el->suivant = *p;
    el->noeud = *a;
    el->valeur = val;
    el->poids = poi;
    *p = el;
}

//depile le dernier noeud et récupère les valeurs (poids, valeur) à ce moment
Arbre depiler(PileNoeud * p, int * poi, int * val) {
    Arbre tmp = (*p)->noeud;
    *poi = (*p)->poids;
    *val = (*p)->valeur;
    *p = (*p)->suivant;
    return tmp;
}

//ajout d'un objet (poids et valeur) par rapport à son efficacité (valeur/poids)
void ajout(ListeObjets * l, int poi, int val){
    ListeObjets o, tmp = *l;
    o = malloc(sizeof(struct objet));
    o->poids = poi;
    o->valeur = val;
    
    float efficacite = (float)val/(float)poi;
    
    if ((*l)==NULL || efficacite >= ((float)(*l)->valeur / (float)(*l)->poids)) {
        o->suivant = *l;
        *l = o;
    } else {
        while((tmp->suivant != NULL) && (efficacite < ((float)tmp->suivant->valeur / (float)tmp->suivant->poids)))
            tmp = tmp->suivant;
        if(tmp->suivant == NULL) {//fin de la liste = pire efficacité
            o->suivant = NULL;
            tmp->suivant = o;
        }
        else { //cas général
            o->suivant = tmp->suivant;
            tmp->suivant = o;
        }
    }
}


void proprieteNoeud(ListeObjets o){
    printf("Objet : valeur %d, poids %d \n", o->valeur, o->poids);
}


void lectureFichier(ListeObjets * l){//lecture du fichier ligne par ligne
    FILE *data;
    
    //ouverture du fichier
    if((data = fopen("C:/User/Gustave T Waldron/Knapsack/data.txt","r")) == NULL) {
        fprintf(stderr, "Echec ouverture fichier. Fin de programme.\n");
        exit(1);
    }
    
    int i=1, val , poids;
    char c;
    while((c=fgetc(data))!=EOF){
        fseek(data, -1, SEEK_CUR); //replace le curseur 1 fois vers l'arriere, à cause de la lecture du premier caractere
        fscanf(data, "%d %d", &poids, &val);
        printf("Objet %d, poids %d, val %d\n", i, poids, val);
        
        ajout(l, poids, val); //ajout de l'objet dans liste triée par efficacité
        
        i++;
        fseek(data, 1, SEEK_CUR); //se deplace de +1 vers l'avant, c'est a dire vers la ligne suivante
        
        //lecture peut-etre inefficace, trouver un moyen de ne pas utiliser fseek pour repositioner le curseur
    }
    fclose(data);
}


 //****************\\
//       ALGO       \\
\\******************//

void etape2(ListeObjets * l, int * bestVal, Arbre a, int b, int valeurInit) { //parcours branche
    ListeObjets tmpl = *l;
    
    Arbre tmpa = a;
    int m = valeurInit;
    
    //Arbre k = NULL; //futur frère
    //ListeObjets fl = NULL; //futur objet de départ
    
    PileNoeud p = NULL;//pile de noeud à parcourir
    
    while (tmpl != NULL) {
        Arbre tmpn = malloc(sizeof(struct noeud)); //création nouveau noeud pour l'arbre
        tmpn->fils = NULL;
        tmpn->frere = NULL;
        tmpn->nbObjet = b / tmpl->poids; //calcul nombre d'objet possible (troncature)
        tmpn->lobjet = tmpl;
        
        b = b - tmpn->nbObjet * tmpl->poids; //mise a jour de la capacité du sac
        m = m + tmpn->nbObjet * tmpl->valeur; //mise a jour de la valeur dans le sac
        
        if (tmpn->nbObjet > 0) {
            /*k = tmpn;
             fl = tmpl; *///fl = k->lobjet;
            empiler(&p, &tmpn, b, m);
        }
        
        if (a->fils == NULL) { //premier élément dans l'arbre
            a->fils = tmpn;
            tmpa = a->fils;
        }
        else { //suite
            tmpa->fils = tmpn;
            tmpa = tmpa->fils;
        }
        tmpl = tmpl->suivant;
    }
    
    if(m > *bestVal)
        *bestVal=m;
    
    while (p != NULL) {//k != NULL && fl != NULL) {
        Arbre k = depiler(&p, &b, &m);
        
        if(k->lobjet->suivant != NULL) {
            float util = (float)m + ((float)(k->lobjet->suivant->valeur) / (float)(k->lobjet->suivant->poids)) * (float)b;
            float best = (float)(*bestVal+1);
            if(util > best) {
                
                b = b + 1 * k->lobjet->poids;
                m = m - 1 * k->lobjet->valeur;
                
                Arbre frere = malloc(sizeof(struct noeud));
                frere->fils = NULL;
                frere->frere = NULL;
                frere->lobjet = k->lobjet;//fl;
                frere->nbObjet = k->nbObjet - 1;
                
                empiler(&p, &frere, b, m);
                
                //etape2(&fl->suivant, bestVal, frere, b + 1 * fl->poids, m - 1 * fl->valeur); //analyse de branche parallele = frère
                etape2(&k->lobjet->suivant, bestVal, frere, b, m);
                
                k->frere = frere; //ajout nouvelle branche dans l'arbre
            }
        }
    }
}

int main(){
    ListeObjets l = NULL;
    
    //ajout des objets dans la liste l
    lectureFichier(&l);
    
    
    //création arbre de solution
    Arbre a = malloc(sizeof(struct noeud));
    a->fils = NULL;
    a->frere = NULL;
    
    int M = 0; //meilleur valeur du sac
    int capaciteSac = 120;
    
    etape2(&l, &M, a, capaciteSac, 0); //première branche 3,0,0,0 ; M=12
    
    printf("Valeur = %d\n", M);
    
    return 0;
}
