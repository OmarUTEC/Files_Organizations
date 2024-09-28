# PROYECTO 1: ORGANIZACIÓN DE ARCHIVOS
## Tabla de Contenidos
- [Introducción](#introducción)
- [Objetivo](#objetivo)
  - [Objetivo General](#objetivo-general)
  - [Objetivo Específico](#objetivo-específico)
- [Dominio de Datos](#dominio-de-datos)
- [Resultados Esperados al Aplicar Técnicas de Indexación](#resultados-esperados-al-aplicar-técnicas-de-indexación)
- [Técnicas de Indexación](#técnicas-de-indexación)
  - [AVL File Organization](#avl-file-organization)
  - [Sequential File Organization](#sequential-file-organization)
  - [Extendible Hashing](#extendible-hashing)
- [Parser SQL](#sql-parser)
- [Resultados  Experimentales](#resultados-experimentales)
  - [Cuadro comparativo del desempeño de las técnicas](#Cuadro-comparativo-del-desempeño-de-las-tecnicas-de-indexación)
  - [Discusión y Análisis de Resultados](#Discusión-y-análisis-de-resultados)

## Introducción
Este proyecto se centra en la integración de datos estructurados y no estructurados, y en el desarrollo de un mini gestor de bases de datos que facilitará la comprensión y la aplicación de técnicas de organización e indexación de archivos.

### OBJETIVO 
`OBJETIVO GENERAL:`  

  Desarrollar un mini sistema de gestor base de datos que integre tanto un modelo relacional basado en 
  tablas como técnicas de recuperación de información. Se aplicarán 3 técnicas de organización de arvhivos: 
  AVL File Organization, Sequential File Organization y Extendible Hashing
  
`OBJETIVO ESPECÍFICO:`  
  - Implementar las 3 técnicas de organización e indexación para desarrollar un mini gestor
  - Implementar las operaciones de búsqueda, búsqueda por rango, añadir y remover
  - Implementar un parser de código SQL
  - Mostrar los resultados de forma interactiva

### DOMINIO DE DATOS
Elegimos 2 datasets: `coaches.csv` y `athletes.csv`  
Hace un dominio de datos conformados por:

| **int**           | 
|----------------------|
| **char**         |


### RESULTADOS ESPERADOS AL APLICAR TÉCNICAS DE INDEXACIÓN
- Optimizar el tiempo de búsqueda al aplicar el índice B+ Tree y el hashing.
- Optimización del Almacenamiento
- Facilidad de inserción y eliminación
- Acceso eficiente a Rangos de Datos
- Facilitación de consultas SQL

## TÉCNICAS DE INDEXACIÓN - ALGORITMO

1. `AVL File Organization`

   Estructura de datos de árbol binario de búsqueda que mantiene un equilibrio entre sus subárboles, lo que asegura que las operaciones de búsqueda, inserción y eliminación se realicen em un tiempo promedio de O(log n). Es decir combina la eficiencia de un árbol binario de búsqueda con la garantía de un rendimiento equilibrio.

   - `Inserción`
     
   - `Búsqueda`
   - `Eliminación`

   
   
2. `Sequential File Organization`
   
   Organiza los datos en un orden lineal, donde los registros se almacenan uno tras otro en un orden lineal, donde los registros se almacenan uno tras otro. Los registros se almacenan en un orden específico, que puede ser determinado por una clave primaria.

   - `Inserción`
   - `Búsqueda`
   - `Eliminación`
3. `Extendible Hashing` 
   
   Utiliza una tabla de hash para organizar y acceder a registros de manera eficiente. Está compuesta por un conjunto de **páginas** que contienen punteros a los registros. Cuando la tabla se llena, se puede expandir mediante la creación de nuevas páginas. Al principio solo se utilizan algunos bits de la clave para indexar. A medida que se añaden más registros y se producen colisiones, se pueden usar más bits para dividir las páginas.

   - `Inserción`
     Se utilizará la secuencia D-bit para localizar el bucket correspondiente. Si no se encuentra el             bucket, se procederá a buscar uno con la profundidad local mínima. En caso de que no se encuentre un        bucket adecuado, se creará uno nuevo. Si se encuentra un bucket que no está lleno, se procederá a           insertar el registro. Si el bucket está lleno, se dividirá y se reinsertarán todos los registros en la      nueva estructura. Si ya no es posible incrementar la profundidad, se optará por el encadenamiento para      manejar el desbordamiento.
     ```cpp
     

    
   - `Búsqueda`
     Se aplicará la función hash a la clave de búsqueda para obtener la secuencia D-bit. Luego, esta secuencia D-bit se utilizará para hacer coincidir con una entrada en el directorio, lo que nos permitirá acceder al bucket correspondiente y localizar el registro deseado.
     
   - `Eliminación`
     Localizar el buffer y eliminamos el registro, si el bucket queda vacío, puede ser liberado, si es que tienen pocos elementos se procede a mezclar. En ambos casos actualizamos el índice.

     

### ANÁLISIS COMPARATIVO
`En base a los accesos en memoria secundaria`


| **Técnica de Organización**      | **Inserción** | **Búsqueda** | **Eliminación** |  **Explicación de Complejidad**                                                                             | **Ventajas**                                  | **Desventajas**                              |
|----------------------------------|---------------|---------------|------------------|------------------------------------------------------------------------------------------------------------|------------------------------------------------|----------------------------------------------|
| **AVL File Organization**        | O(log n)      | O(log n)      | O(log n)         | La altura es logarítmica y requiere rotaciones para mantener el equilibrio tras inserciones o eliminaciones. | Balanceo automático; acceso rápido.          | Complejidad en la implementación y mantenimiento. |
| **Sequential File Organization** | O(log n)          | O(log n)          | O(n)             | Se debe recorrer el archivo desde el inicio para encontrar registros, lo que es ineficiente en archivos grandes. | Sencillo de implementar; eficiente en lectura secuencial. | Ineficiente para búsquedas y modificaciones. |
| **Extendible Hashing**           | O(1)          | O(1)          | O(1)             | Se usa la función hash para acceder directamente al bucket. Las colisiones pueden requerir ajustes, pero en promedio sigue siendo eficiente. | Acceso rápido; bajo costo de inserción y búsqueda. | Complejidad en el manejo de colisiones y ajuste de la tabla. |


## SQL PARSER
La implementación del parser para consultas SQL se ha llevado a cabo siguiendo un enfoque escalable. En primer lugar, hemos desarrollado un scanner el cual se encarga de tokenizar los lexemas de una cadena de consulta SQL

## RESULTADOS EXPERIMENTALES

### CUADRO COMPARATIVO DEL DESEMPEÑO DE LAS TÉCNICAS DE INDEXACIÓN
`Inserción` `Búsqueda`

### DISCUSIÓN Y ANÁLISIS DE LOS RESULTADOS
