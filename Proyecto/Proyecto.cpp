// Proyecto.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#include "Proyecto.h"
#include <string>
#include <ctime>
#include <fstream>
#include <windows.h>
#include <commctrl.h>  // Para el DateTimePicker
#pragma comment(lib, "Comctl32.lib")  // Linkear la librería
#include "ArbolProductos.h"

#define MAX_LOADSTRING 100

using namespace std;

// Variables globales:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Declaraciones de funciones adelantadas incluidas en este módulo de c0digo:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Registro(HWND, UINT, WPARAM, LPARAM);

struct ProductoInventario {
    string nombre;
    double precioCompra;
    int cantidad;
    string descripcion;
    ProductoInventario* siguiente;

    // Constructor que inicializa todos los miembros
    ProductoInventario(const string& nombre = "", double precioCompra = 0.0, int cantidad = 0,
        const string& descripcion = "", ProductoInventario* siguiente = nullptr)
        : nombre(nombre), precioCompra(precioCompra), cantidad(cantidad),
        descripcion(descripcion), siguiente(siguiente) {}
};

// Declarar la raíz del árbol binario de productos
NodoProducto* raiz = nullptr;


// Estructura para el usuario
struct nodo_usuario {
    string nombre;
    string contra;
    tm fecha_nacimiento;
    double dinero;
    ProductoInventario* inventario = nullptr;
    nodo_usuario* siguiente = nullptr;

    // Constructor que inicializa todos los miembros
    nodo_usuario(const string& nombre = "", const string& contra = "", const tm& fecha_nacimiento = {},
        double dinero = 0.0)
        : nombre(nombre), contra(contra), fecha_nacimiento(fecha_nacimiento), dinero(dinero),
        inventario(nullptr), siguiente(nullptr) {}
};

// Puntero a la lista de usuarios
nodo_usuario* lista_usuarios = nullptr;
// Variable global para almacenar el usuario actual
nodo_usuario* usuario_actual = nullptr;
void guardar_todos_los_usuarios();

// Función para convertir wchar_t a std::string
string convertir_wchar_a_string(const wchar_t* wstr) {
    int buffer_size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    string str(buffer_size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], buffer_size, nullptr, nullptr);
    return str;
}

// Estructura para representar un producto
struct Producto {
    string nombre;
    double precio;
    int cantidadDisponible;
};


//agregarProductoAlArbol llenarListBoxProductos lo que uso para agregar productos al arbol
// void inicializarProductos es para crear los productos disponibles


// Lista de productos (A, B, C) inicializados
Producto productos[] = {
    {"Producto A", 10.0, 20},  // Nombre, Precio, Cantidad
    {"Producto B", 15.0, 15},
    {"Producto C", 20.0, 10}
};

// Cantidad de productos en la tienda
const int numProductos = sizeof(productos) / sizeof(productos[0]);


// Función para verificar si un nombre de usuario ya existe
bool nombre_usuario_existe(const string& nombre) {
    nodo_usuario* actual = lista_usuarios;
    while (actual != nullptr) {
        if (actual->nombre == nombre) {
            return true; // El nombre de usuario ya existe
        }
        actual = actual->siguiente;
    }
    return false; // El nombre de usuario no existe
}

// Función para autenticar un usuario basado en nombre y contraseña
bool autenticar_usuario(const string& nombre, const string& contra) {
    nodo_usuario* actual = lista_usuarios;
    while (actual != nullptr) {
        if (actual->nombre == nombre && actual->contra == contra) {
            return true; // Autenticación exitosa
        }
        actual = actual->siguiente;
    }
    return false; // Usuario o contraseña incorrecta
}

// Recuperar usuarios con el archivo binario o crearlo si no existe
void recuperar_usuarios_bin() {
    ifstream archivo("usuarios.bin", ios::in | ios::binary);

    // Verificar si el archivo no existe
    if (!archivo.is_open()) {
        // Crear el archivo binario si no existe
        ofstream nuevo_archivo("usuarios.bin", ios::out | ios::binary);
        if (nuevo_archivo.is_open()) {
            MessageBox(NULL, L"El archivo 'usuarios.bin' no existía y se ha creado correctamente.", L"Archivo Creado", MB_OK | MB_ICONINFORMATION);
            nuevo_archivo.close();
        }
        else {
            MessageBox(NULL, L"Error al crear el archivo 'usuarios.bin'.", L"Error", MB_OK | MB_ICONERROR);
        }
        return;
    }

    // Limpiar lista actual de usuarios
    while (archivo.peek() != EOF) {
        nodo_usuario* usuario_recuperado = new nodo_usuario;
        archivo.read(reinterpret_cast<char*>(usuario_recuperado), sizeof(nodo_usuario));

        // Verificar si la lectura fue exitosa
        if (archivo.gcount() != sizeof(nodo_usuario)) {
            delete usuario_recuperado;  // Liberar memoria si la lectura falla
            break;
        }

        // Validar los datos recuperados (por ejemplo, verificar que el nombre no esté vacío)
        if (usuario_recuperado->nombre.empty()) {
            delete usuario_recuperado; // Si el nombre está vacío, algo salió mal
            continue;
        }

        // Añadir a la lista de usuarios
        usuario_recuperado->siguiente = lista_usuarios;
        lista_usuarios = usuario_recuperado;
    }
    archivo.close();
}


// Función para registrar un nuevo usuario y guardarlo en archivo binario
void registrar_usuario(const string& nombre, const string& contra, const tm& fecha_nacimiento, double dinero_inicial) {
    // Verificar si el nombre de usuario ya existe
    if (nombre_usuario_existe(nombre)) {
        MessageBox(NULL, L"El nombre de usuario ya existe. Elija otro.", L"Error de registro", MB_OK | MB_ICONERROR);
        return;
    }

    nodo_usuario* nuevo_usuario = new nodo_usuario;
    nuevo_usuario->nombre = nombre;
    nuevo_usuario->contra = contra;
    nuevo_usuario->fecha_nacimiento = fecha_nacimiento;
    nuevo_usuario->dinero = dinero_inicial;  // Asignar el dinero inicial a la cuenta del usuario

    // Añadir el nuevo usuario a la lista
    nuevo_usuario->siguiente = lista_usuarios;
    lista_usuarios = nuevo_usuario;

    // Guardar en archivo binario
    ofstream archivo("usuarios.bin", ios::out | ios::binary | ios::app);
    if (archivo.is_open()) {
        archivo.write(reinterpret_cast<char*>(nuevo_usuario), sizeof(nodo_usuario));
        archivo.close();
        MessageBox(NULL, L"Usuario registrado con exito", L"Registro", MB_OK);
    }
    else {
        MessageBox(NULL, L"Error al guardar el usuario", L"Error", MB_OK | MB_ICONERROR);
    }
}

// Función para buscar un usuario por nombre y contraseña
nodo_usuario* buscar_usuario(const string& nombre, const string& contra) {
    nodo_usuario* actual = lista_usuarios;
    while (actual != nullptr) {
        if (actual->nombre == nombre && actual->contra == contra) {
            return actual;  // Devolver el usuario si la autenticación es exitosa
        }
        actual = actual->siguiente;
    }
    return nullptr;  // Usuario no encontrado o contraseña incorrecta
}


// Función para manejar el diálogo de registro de usuarios
INT_PTR CALLBACK Registro(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            // Obtener los datos del nombre de usuario y contraseña
            wchar_t nombre[100], contra[100], dinero[100];
            GetDlgItemText(hDlg, IDC_NOM, nombre, 100);
            GetDlgItemText(hDlg, IDC_CONTRA, contra, 100);
            GetDlgItemText(hDlg, IDC_DINERO, dinero, 100);  // Leer el valor del dinero desde el edit control

            // Validar que el valor de dinero sea numérico
            wstring dinero_wstr(dinero);
            bool es_numero_valido = true;
            for (wchar_t c : dinero_wstr) {
                if (!iswdigit(c) && c != L'.') {  // Permitir solo dígitos y un punto decimal
                    es_numero_valido = false;
                    break;
                }
            }

            if (!es_numero_valido) {
                MessageBox(hDlg, L"Por favor, ingrese un valor numérico valido para el dinero.", L"Error de entrada", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;  // Evitar el cierre del diálogo si la entrada es inválida
            }

            // Convertir el valor de dinero a un tipo double
            double dinero_inicial = _wtof(dinero);

            // Validar que el dinero sea mayor o igual a cero y esté dentro de un límite razonable
            if (dinero_inicial < 0 || dinero_inicial > 10000) {
                MessageBox(hDlg, L"Por favor, ingrese un valor de dinero entre 0 y 10,000.", L"Error de entrada", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;  // Evitar el cierre del diálogo si el valor está fuera de rango
            }

            // Obtener la fecha de nacimiento del DateTimePicker
            SYSTEMTIME fecha;
            tm fecha_nacimiento = {};
            DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER1), &fecha);
            fecha_nacimiento.tm_mday = fecha.wDay;
            fecha_nacimiento.tm_mon = fecha.wMonth - 1;  // Enero es 0
            fecha_nacimiento.tm_year = fecha.wYear - 1900; // Contado desde 1900

            // Convertir los valores de wchar_t a std::string
            string nombre_str = convertir_wchar_a_string(nombre);
            string contra_str = convertir_wchar_a_string(contra);

            // Llamar a la función de registro con el nuevo dato de dinero
            registrar_usuario(nombre_str, contra_str, fecha_nacimiento, dinero_inicial);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// Función para manejar el diálogo de inicio de sesión de usuarios
INT_PTR CALLBACK Ingresar(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {  // Cuando se hace clic en el botón de aceptar
            wchar_t nombre[100], contra[100];
            GetDlgItemText(hDlg, IDC_NOM, nombre, 100);
            GetDlgItemText(hDlg, IDC_CONTRA, contra, 100);

            // Convertir los valores de wchar_t a std::string
            string nombre_str = convertir_wchar_a_string(nombre);
            string contra_str = convertir_wchar_a_string(contra);

            // Llamar a la función de autenticación y establecer el usuario actual si es exitoso
            nodo_usuario* usuario_encontrado = buscar_usuario(nombre_str, contra_str);
            if (usuario_encontrado != nullptr) {
                usuario_actual = usuario_encontrado;  // Establecer el usuario actual autenticado
                MessageBox(NULL, L"Inicio de sesi0n exitoso", L"Autenticacion", MB_OK);
                EndDialog(hDlg, LOWORD(wParam)); // Cierra el diálogo después del éxito
            }
            else {
                MessageBox(NULL, L"Nombre de usuario o contrasenia incorrectos", L"Error", MB_OK | MB_ICONERROR);
            }
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam)); // Cerrar el diálogo y volver al menú
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



// Función para agregar dinero a la cuenta del usuario
void agregar_dinero_a_usuario(nodo_usuario* usuario, double cantidad) {
    if (cantidad <= 0) {
        MessageBox(NULL, L"La cantidad debe ser un numero positivo.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Actualizar el saldo del usuario
    usuario->dinero += cantidad;

    // Actualizar el archivo binario para reflejar el nuevo saldo
    guardar_todos_los_usuarios();
    MessageBox(NULL, L"Dinero agregado exitosamente a la cuenta.", L"Transaccion Exitosa", MB_OK | MB_ICONINFORMATION);
}

// Función para sobrescribir el archivo binario con la lista completa de usuarios
void guardar_todos_los_usuarios() {
    ofstream archivo("usuarios.bin", ios::out | ios::binary); // Modo de escritura estándar (sobrescribe todo)

    if (archivo.is_open()) {
        nodo_usuario* actual = lista_usuarios;
        while (actual != nullptr) {
            archivo.write(reinterpret_cast<char*>(actual), sizeof(nodo_usuario));
            actual = actual->siguiente;
        }
        archivo.close();
    }
    else {
        MessageBox(NULL, L"Error al guardar los datos del usuario.", L"Error", MB_OK | MB_ICONERROR);
    }
}


// Función para agregar un producto al inventario del usuario
void agregar_a_inventario(nodo_usuario* usuario, const string& nombreProducto, double precio, int cantidad, const string& descripcion) {
    if (usuario == nullptr) return; // Validación de usuario

    // Crear un nuevo producto para el inventario
    ProductoInventario* nuevoProducto = new ProductoInventario;
    nuevoProducto->nombre = nombreProducto;
    nuevoProducto->precioCompra = precio;
    nuevoProducto->cantidad = cantidad;
    nuevoProducto->descripcion = descripcion;

    // Añadir al inventario del usuario (al inicio de la lista)
    nuevoProducto->siguiente = usuario->inventario;
    usuario->inventario = nuevoProducto;
}



// Función para ver el saldo
INT_PTR CALLBACK VerSaldo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // Inhabilitar el Edit Control para que sea solo de lectura
        EnableWindow(GetDlgItem(hDlg, IDC_SALDO_ACTUAL), FALSE);

        // Mostrar el saldo del usuario autenticado en el Edit Control
        if (usuario_actual != nullptr) {
            wchar_t saldoTexto[50];
            swprintf(saldoTexto, 50, L"$%.2f", usuario_actual->dinero);
            SetDlgItemText(hDlg, IDC_SALDO_ACTUAL, saldoTexto);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Funcion para manejar el dialog de agregar dinero a la cuenta del usuario
INT_PTR CALLBACK AgregarDinero(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            wchar_t dinero[100];
            GetDlgItemText(hDlg, IDC_DINERO, dinero, 100);

            // Validar que el valor de dinero sea numérico
            double cantidad = _wtof(dinero);
            if (cantidad <= 0) {
                MessageBox(hDlg, L"Ingrese una cantidad valida de dinero mayor a 0.", L"Error de entrada", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            // Llamar a la funcion para agregar dinero usando la variable global "usuario_actual"
            agregar_dinero_a_usuario(usuario_actual, cantidad);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Función original para llenar el list box
//void llenarListBoxProductos(NodoProducto* nodo, HWND hDlg, int listBoxID) {
//    if (nodo == nullptr) return;
//
//    // Llamada recursiva al nodo anterior (izquierdo)
//    llenarListBoxProductos(nodo->ant, hDlg, listBoxID);
//
//    // Agregar el nombre del producto al ListBox
//    SendDlgItemMessage(hDlg, listBoxID, LB_ADDSTRING, 0, (LPARAM)nodo->nombre.c_str());
//
//    // Llamada recursiva al nodo siguiente (derecho)
//    llenarListBoxProductos(nodo->sig, hDlg, listBoxID);
//}
// Función para llenar el ListBox con los productos desde el árbol binario

void llenarListBoxProductos(NodoProducto* nodo, HWND hDlg, int listBoxID) {
    if (nodo == nullptr) return;

    llenarListBoxProductos(nodo->ant, hDlg, listBoxID);
    SendDlgItemMessage(hDlg, listBoxID, LB_ADDSTRING, 0, (LPARAM)nodo->nombre.c_str());
    llenarListBoxProductos(nodo->sig, hDlg, listBoxID);
}


// Función para buscar un producto en el árbol binario
NodoProducto* buscarProductoEnArbol(NodoProducto* nodo, const string& nombreProducto) {
    if (nodo == nullptr) return nullptr;

    if (nodo->nombre == nombreProducto) {
        return nodo; // Producto encontrado
    }
    else if (nombreProducto < nodo->nombre) {
        return buscarProductoEnArbol(nodo->ant, nombreProducto); // Buscar en el lado izquierdo
    }
    else {
        return buscarProductoEnArbol(nodo->sig, nombreProducto); // Buscar en el lado derecho
    }
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROYECTO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Inicializar common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_DATE_CLASSES;
    InitCommonControlsEx(&icex);

    // Recuperar los usuarios desde el archivo binario
    recuperar_usuarios_bin();

    // Realizar la inicialización de la aplicación:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROYECTO));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROYECTO));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROYECTO);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Almacenar identificador de instancia en una variable global

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Función para convertir string a wstring (wchar_t)
wstring convertirAUnicode(const string& texto) {
    int tamNecesario = MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), -1, NULL, 0);
    wstring resultado(tamNecesario, 0);
    MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), -1, &resultado[0], tamNecesario);
    return resultado;
}

INT_PTR CALLBACK CompraProducto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static double dineroUsuario = usuario_actual ? usuario_actual->dinero : 0.0;
    static string productoSeleccionado;
    wchar_t nombreProductoW[100] = { 0 }; // Inicializar con ceros para evitar basura en memoria

    switch (message) {
    case WM_INITDIALOG:
        // Validación: Si el usuario no ha iniciado sesión, cerrar el diálogo
        if (usuario_actual == nullptr) {
            MessageBox(hDlg, L"No has iniciado sesión. Por favor, ingresa primero.", L"Error de autenticación", MB_OK | MB_ICONERROR);
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }

        // Poblar el ListBox con la lista de productos desde el árbol binario
        llenarListBoxProductos(raiz, hDlg, IDC_PRODUCTO_LISTA);

        // Seleccionar el primer producto por defecto (si existe)
        SendDlgItemMessage(hDlg, IDC_PRODUCTO_LISTA, LB_SETCURSEL, 0, 0);

        // Inhabilitar el Edit Control de total para que sea solo de lectura
        EnableWindow(GetDlgItem(hDlg, IDC_TOTAL), FALSE);
        SetDlgItemText(hDlg, IDC_TOTAL, L"$0.00");
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: // Botón Aceptar
        {
            // Obtener la cantidad ingresada
            int cantidad = GetDlgItemInt(hDlg, IDC_CANTIDAD, NULL, FALSE);
            if (cantidad <= 0) {
                MessageBox(hDlg, L"Por favor, ingrese una cantidad válida.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            // Obtener el producto seleccionado en el ListBox
            LRESULT index = SendDlgItemMessage(hDlg, IDC_PRODUCTO_LISTA, LB_GETCURSEL, 0, 0);
            if (index == LB_ERR) {
                MessageBox(hDlg, L"No se ha seleccionado un producto.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            // Obtener el nombre del producto desde el ListBox
            if (SendDlgItemMessage(hDlg, IDC_PRODUCTO_LISTA, LB_GETTEXT, index, (LPARAM)nombreProductoW) == LB_ERR) {
                MessageBox(hDlg, L"Error al obtener el nombre del producto.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            // Convertir el nombre de wchar_t a string
            string nombreProducto = convertir_wchar_a_string(nombreProductoW);

            // Buscar el producto en el árbol binario
            NodoProducto* producto = buscarProductoEnArbol(raiz, nombreProducto);
            if (producto == nullptr) {
                MessageBox(hDlg, L"Producto no encontrado.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            // Calcular el costo total de la compra
            double costoTotal = cantidad * producto->precioBase;

            // Verificar inventario en la tienda y saldo del usuario
            if (cantidad > producto->cantidadDisponible) {
                MessageBox(hDlg, L"No hay suficiente inventario en la tienda.", L"Error", MB_OK | MB_ICONERROR);
            }
            else if (costoTotal > usuario_actual->dinero) {
                MessageBox(hDlg, L"No tienes suficiente dinero.", L"Saldo Insuficiente", MB_OK | MB_ICONERROR);
            }
            else {
                // Compra exitosa: reducir inventario en la tienda y actualizar el saldo del usuario
                producto->cantidadDisponible -= cantidad;
                usuario_actual->dinero -= costoTotal;

                // Agregar el producto al inventario del usuario
                agregar_a_inventario(usuario_actual, producto->nombre, producto->precioBase, cantidad, "Descripción");

                // Guardar los cambios en el archivo binario
                guardar_todos_los_usuarios();

                MessageBox(hDlg, L"Compra realizada con éxito. Producto añadido a tu inventario.", L"Compra Exitosa", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK);
            }
            return (INT_PTR)TRUE;
        }

        case IDCANCEL: // Botón Cancelar
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;

        case IDC_PRODUCTO_LISTA: // Cambio en la selección del ListBox
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                LRESULT index = SendDlgItemMessage(hDlg, IDC_PRODUCTO_LISTA, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    // Actualizar la información del producto seleccionado
                    if (SendDlgItemMessage(hDlg, IDC_PRODUCTO_LISTA, LB_GETTEXT, index, (LPARAM)nombreProductoW) != LB_ERR) {
                        string nombreProducto = convertir_wchar_a_string(nombreProductoW);
                        NodoProducto* producto = buscarProductoEnArbol(raiz, nombreProducto);
                        if (producto != nullptr) {
                            wchar_t infoProducto[200];
                            swprintf(infoProducto, 200, L"Nombre: %s\nUnidades: %d\nPrecio: $%.2f",
                                convertirAUnicode(producto->nombre).c_str(),
                                producto->cantidadDisponible,
                                producto->precioBase);
                            SetDlgItemText(hDlg, IDC_DETALLES, infoProducto);
                        }
                    }
                }
            }
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// Función para buscar un producto en el inventario del usuario
ProductoInventario* buscarProductoEnInventario(nodo_usuario* usuario, const string& nombreProducto) {
    if (usuario == nullptr) return nullptr;

    ProductoInventario* actual = usuario->inventario;
    while (actual != nullptr) {
        if (actual->nombre == nombreProducto) {
            return actual; // Producto encontrado
        }
        actual = actual->siguiente;
    }
    return nullptr; // Producto no encontrado
}


// Función para eliminar un producto del inventario del usuario
void eliminarProductoDeInventario(nodo_usuario* usuario, const string& nombreProducto) {
    if (usuario == nullptr) return;

    ProductoInventario* actual = usuario->inventario;
    ProductoInventario* anterior = nullptr;

    // Buscar el producto en la lista
    while (actual != nullptr) {
        if (actual->nombre == nombreProducto) {
            // Si encontramos el producto, lo eliminamos
            if (anterior == nullptr) {
                // Si es el primer producto en la lista
                usuario->inventario = actual->siguiente;
            }
            else {
                // Si es un producto en medio o al final de la lista
                anterior->siguiente = actual->siguiente;
            }
            delete actual; // Liberar la memoria
            return;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
}

INT_PTR CALLBACK MostrarInventario(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static ProductoInventario* productoSeleccionado = nullptr;
    ProductoInventario* producto = nullptr;

    switch (message) {
    case WM_INITDIALOG:
        if (usuario_actual == nullptr) {
            MessageBox(hDlg, L"No has iniciado sesión. Por favor, ingresa primero.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }

        producto = usuario_actual->inventario;
        while (producto != nullptr) {
            SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_ADDSTRING, 0, (LPARAM)producto->nombre.c_str());
            producto = producto->siguiente;
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INVENTARIO_LISTA:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int index = SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_GETCURSEL, 0, 0);

                // Validar si el índice es válido
                if (index == LB_ERR) {
                    MessageBox(hDlg, L"No se ha seleccionado un producto.", L"Error", MB_OK | MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }

                // Obtener la longitud del texto
                int textLength = SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_GETTEXTLEN, index, 0);

                // Validar longitud del texto
                if (textLength <= 0 || textLength >= 100) {
                    MessageBox(hDlg, L"Error al obtener el nombre del producto.", L"Error", MB_OK | MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }

                wchar_t nombreProductoW[100] = { 0 };

                // Obtener el texto del producto seleccionado
                if (SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_GETTEXT, index, (LPARAM)nombreProductoW) == LB_ERR) {
                    MessageBox(hDlg, L"Error al leer el nombre del producto.", L"Error", MB_OK | MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }

                string nombreProducto = convertir_wchar_a_string(nombreProductoW);

                // Asegurarnos de que `productoSeleccionado` sea válido
                productoSeleccionado = buscarProductoEnInventario(usuario_actual, nombreProducto);
                if (productoSeleccionado != nullptr) {
                    wchar_t detalles[200];
                    swprintf(detalles, 200, L"Nombre: %s\nCantidad: %d\nPrecio de Compra: $%.2f",
                        convertirAUnicode(productoSeleccionado->nombre).c_str(),
                        productoSeleccionado->cantidad,
                        productoSeleccionado->precioCompra);
                    SetDlgItemText(hDlg, IDC_DETALLES_PRODUCTO, detalles);
                }
                else {
                    SetDlgItemText(hDlg, IDC_DETALLES_PRODUCTO, L"Producto no encontrado.");
                }
            }
            break;

        case ID_VENDER_PRODUCTO:
            if (productoSeleccionado != nullptr) {
                eliminarProductoDeInventario(usuario_actual, productoSeleccionado->nombre);
                guardar_todos_los_usuarios();

                // Limpiar y actualizar el ListBox después de la venta
                SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_RESETCONTENT, 0, 0);
                producto = usuario_actual->inventario;
                while (producto != nullptr) {
                    SendDlgItemMessage(hDlg, IDC_INVENTARIO_LISTA, LB_ADDSTRING, 0, (LPARAM)producto->nombre.c_str());
                    producto = producto->siguiente;
                }
                SetDlgItemText(hDlg, IDC_DETALLES_PRODUCTO, L"");
                MessageBox(hDlg, L"Producto vendido con éxito.", L"Venta Exitosa", MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(hDlg, L"No has seleccionado un producto para vender.", L"Error", MB_OK | MB_ICONERROR);
            }
            break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Analizar las selecciones de menú:
        switch (wmId)
        {//ID_COMPRA_COMPRAR
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case ID_USUARIO_REGISTRARSE:  // Evento para el menú de registro
            DialogBox(hInst, MAKEINTRESOURCE(IDD_REGISTRO), hWnd, Registro);
            break;
        case ID_USUARIO_INGRESAR:  // Evento para el menú de inicio de sesión
            DialogBox(hInst, MAKEINTRESOURCE(IDD_INGRESAR), hWnd, Ingresar);
            break;
        case ID_USUARIO_AGREGAR_DINERO:  // Evento para el menú de agregar dinero
            if (usuario_actual != nullptr) {  // Verificar que el usuario esté autenticado antes de agregar dinero
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AGREGAR_DINERO), hWnd, AgregarDinero, reinterpret_cast<LPARAM>(usuario_actual));
            }
            else {
                MessageBox(hWnd, L"No hay ningun usuario autenticado. Inicie sesión para agregar dinero.", L"Error", MB_OK | MB_ICONERROR);
            }
            break;
        case ID_USUARIO_CERRAR_SESION:  // Evento para cerrar sesión
            if (usuario_actual != nullptr) {
                int respuesta = MessageBox(hWnd, L"¿Estas seguro de que deseas cerrar sesión?", L"Confirmacion", MB_YESNO | MB_ICONQUESTION);
                if (respuesta == IDYES) {
                    usuario_actual = nullptr;  // Restablecer el usuario actual
                    MessageBox(hWnd, L"Has cerrado sesión exitosamente.", L"Sesión Cerrada", MB_OK | MB_ICONINFORMATION);
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_INGRESAR), hWnd, Ingresar);  // Mostrar la pantalla de inicio de sesión nuevamente
                }
            }
            else {
                MessageBox(hWnd, L"No hay ningun usuario autenticado.", L"Error", MB_OK | MB_ICONERROR);
            }
            break;
        case ID_COMPRA_ABRIR:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_COMPRA), hWnd, CompraProducto);
            break;
        case ID_USUARIO_VER_SALDO:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VER_SALDO), hWnd, VerSaldo, reinterpret_cast<LPARAM>(usuario_actual));
            break;
        case ID_USUARIO_INVENTARIO:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_INVENTARIO), hWnd, MostrarInventario);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
