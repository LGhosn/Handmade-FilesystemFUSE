# fisop-fs

Lugar para respuestas en prosa y documentación del TP.

Al hacer touch prueba se llama:
[debug] fisopfs_getattr - path: /prueba
[debug] fisopfs_createfiles - path: /prueba
[debug] fisopfs_getattr - path: /prueba
[debug] fisop_release - path: /prueba
[debug] fisopfs_getattr - path: /prueba

Al hacer mkdir mount se llama:
[debug] fisopfs_getattr - path: /mount
[debug] fisop_createdir - path: /mount
[debug] fisopfs_getattr - path: /mount

## Documentación

> Las estructuras en memoria que almacenarán los archivos, directorios y sus metadatos.

#### File System

```c
struct fs {
	size_t size;
	struct dir *root;
};
```

Nuestra estructura del sistema de archivos tiene un campo `size` que tiene la cantidad de entries en el sistema y un puntero a `struct dir` que es la raíz (o directorio root o directorio "/") que tiene a todos los archivos/directorios.

#### Entry

```c
struct entry {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
};
```

`struct entry` es una estructura que contiene los campos compartidos entre `struct file` y `struct dir` para abilitar el tener un arreglo de archivos o directorios (como el polimorfismo del `struct cmd` del tp-shell).

* `name` Tiene el nombre del archivo/directorio. Tiene un máximo de `NAMELEN`.
* `mdata` Una estructura que guarda la metadata del entry.

```c
struct metadata {
	time_t ctime; // create.
	time_t mtime; // modified.
	time_t atime; // accessed.
	uid_t uid;
	gid_t gid;
	mode_t mode;
};
```

La metadata de un inodo en Unix tiene más campos pero estos son los que pensamos eran suficientes para lograr lo pedido en la consigna del tp.

* `type` Un enum que guarda si este entry es un archivo o un directorio.
  
```c
enum entrytype {
	E_FILE,
	E_DIR,
};
```

* `len` Es el largo de la data a guardar, si es un archivo es el largo de su contenido y si es un directorio es la cantidad de entries que guarda.

#### Tipos de entries

```c
struct file {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
	// File specific.
	char *data;
	// Not dumpeable.
	size_t capacity;
};
```

```c
struct dir {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
	// Dir specific.
	struct entry **entries;
	// Not dumpeable.
	size_t capacity;
};
```

Ambas estructuras extienden `struct entry` para lograr su proposito. `struct file` guarda los datos del archivo y `struct dir` un arreglo de `struct entry` los cuales saben de que tipo son porque en ellos se almacena el campo `entry->type`.

> Cómo el sistema de archivos encuentra un archivo específico dado un path

```c
// Dado un path, busca dentro del fs el `struct entry *`
// asociado a él.
struct entry *fs_get(struct fs fs, const char *path);
```

Esta función usa por debajo a otra función:

```c
// Es la implementación de `fs_get`, es de hecho un wrapper
// sin funcionalidad extra.
struct entry *get_entry(struct fs fs, const char *path)
```
`get_entry()` toma el path y llama a `split_path()`.

```c
// Dado un path lo descompone en un arreglo de strings
// y guarda su largo en len.
// Por ejemplo: "/path/to/dir" -> ["path", "to", "dir"].
char **split_path(const char *path, size_t *len);

// Libera la memoria usada por split_path().
void free_split_path(char **split_path);
```

Esta funcionalidad es útil cuando por ejemplo queremos obtener al padre de un entry para agregarle/borrarle otro entry.

Ahora que tenemos un arreglo con los nombres de las entradas que queremos, se itera sobre las entradas de los directorios buscando que coincidan con la posición en el arreglo.

Esto lo hace la función `__get_entry()`:

```c
// Itera recursivamente desde un `struct entry *`. Si encuentra el entry con el path devuelve
// su puntero, si no lo encuentra devuelve NULL.
static struct entry *__get_entry(struct entry *entry, char **split, size_t pos, size_t len);
```

> El formato de serialización del sistema de archivos en disco

#### Persistencia

Nosotros logramos la persistencia del file system haciendo uso de estas funciones:

```c
// Lee un file descriptor y crea un `struct fs`.
struct fs from(int fd);

// Escribe los contenidos del `struct fs` al file descriptor.
void dump(struct fs fs, int fd);
```

Estas funciones hacen uso de otras funciones privadas que son:

```c
// Lee un file descriptor, crea un `struct entry`
// almacenado en el heap y devuelve su direccion de memoria.
static struct entry *read_entry(int fd);

// Escribe los contenidos del `struct entry` en el file descriptor dado.
static void write_entry(int fd, struct entry *entry);
```

Estas otras funciones por debajo escriben los campos del `struct entry` en memoria, hacen uso de un switch del campo `entry->type` para determinar si lo que se esta leyendo/escribiendo es un arreglo de caracteres o un arreglo de más `struct entry` (dependiendo de si es un `E_FILE` o `E_DIR`).

Esta estructura de persistencia permite la serialización y deserialización de un filesystem (que siga nuestra estructura) de hasta cualquier nivel de profundidad.

Al leer el file system del archivo binario, los campos `file->capacity` y `dir->capacity` (al no serializarse) se calculan dependiendo del largo de la data leida. Se utiliza una constante `READCOEF` que determina por cuanto se va a multiplicar el largo actual de la data para pedirle esa capacidad a `malloc` para el campo de data del entry. Por ejemplo si se está levantando un archivo entonces primero se lee el largo de la data y luego se le pedira a `malloc` un bloque de **READCOEF * N** de largo, siendo **N** el largo en bytes de la data actual.

#### Liberación de memoria

Como estas estructuras viven en el heap, su memoria necesita ser liberada. Nosotros liberamos la memoria cuando el file system se escribe en el archivo utilizando la siguiente función:

```c
// Libera la memoria utilizada por un `struct entry`.
static void entry_free(struct entry *entry);
```

Al terminar de serializar un `struct entry`, se llama a esta función que va a hacer free de la estructura. Utiliza el polimorfismo explicado más arriba para determinar si hay que liberar solo un arreglo de data o un arreglo de mas `struct entry` en el caso que sea un directorio. Esto funciona bien porque en el caso de ser un `struct dir`, al llamarse a `entry_free` al final de su serialización, este ya serializó todas sus entries por lo que liberar su memoria es recorrer su arreglo `dir->entries` de largo `dir->len` llamando a `entry_free` por cada uno de ellas.

> Cualquier otra decisión/información que crean relevante

Implementamos unos wrappers de las funciones `malloc` y `calloc` para ayudarnos con el manejo de memoria.

```c
void *xmalloc(size_t s);
void *xcalloc(size_t q, size_t s);
```

Estas funciones hacen lo mismo que sus respectivas `malloc` y `calloc` pero siempre devuelven un puntero válido. En el caso en que `malloc` o `calloc` fallen, `xmalloc` y `xcalloc` van a imprimir por pantalla el error y llamar a `exit` con un valor de `1`.

## TESTS

#### Pruebas de bash

En una consola darle permisos a los test del server y correrlo.
```bash
chmod +x test_sv.sh
./test_sv.sh
```

Luego, en otra consola darle permisos al cliente y correrlo
```bash
chmod +x test_client.sh
./test_client.sh
```

para volver a correr los test repetir ambos pasos.

#### Pruebas de bash de persistencia

Primero hacer las Pruebas de bash y luego correr en una consola
```bash
./test_sv.sh p
```

Luego, en otra consola darle permisos al cliente y correrlo
```bash
chmod +x test_client_per.sh
./test_client_per.sh
```
para volver a correr los test repetir ambos pasos.

#### Pruebas test.c

```bash
chmod +x tests.sh
./tests.sh
```
