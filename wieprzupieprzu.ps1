# Ścieżka do pliku CSV
$csvPath = "C:\temp\AreoTransfinal.csv"

# Odczytujemy danych z pliku CSV
$userData = Import-Csv $csvPath

# Tworzymy OU _Groups, jeżeli nie istnieje
$sciezkaGroups = „DC=AreoTrans,DC=local"

# Zapisujemy atrybuty dla każdego wiersza w pliku CSV do zmiennych
foreach ($user in $userData) {
    $imie = $user.Imię
    $nazwisko = $user.Nazwisko
    $nazwiskoimie = $user.'Nazwisko Imię'
    $przelozony = $user.Przełożony
    $logonName = $user.Login
    $email = $user.Mail
    $id = $user.EmployeeID
    $office = $user.Biuro
    $department = $user.Wydział
    $jobTitle = $user.Stanowisko
    $company = $user.Firma
    $telefon = $user.Telefon
    $miasto = $user.Miasto
    $kraj = "PL"
    $ulica = $user.Ulica
    $budynek = $user.Budynek
    $pomieszczenie = $user.Pomieszenie
    $kod = $user.Kod
    $adres = "$ulica $budynek $pomieszczenie"
    $obszar = $user.Obszar
    $grupa1 = $user.'Grupa 1'
    $grupa2 = $user.'Grupa2'
    $grupa3 = $user.'Grupa3'

    $password = "!1Olsztyn"

    # Ustawiamy zmienną $firma wg zmiennej $company    
    switch ($company) {
    "Grupa AreoTrans" { $firma = "_Zarząd" }
    "Tani Bilet sp. z.o.o." { $firma = "_TaniBilet" }
    "Sprawna przesyłka sp. z o.o." { $firma = "_SprawnaPrzesyłka" }
    "AreoTransFast" { $firma = "_AreoTrans" }
    default { 
        Write-Error "Nieznana firma: $company. Skrypt zostanie przerwany."
        exit 1  # Przerwanie działania skryptu z kodem błędu 1 }  # Domyślna wartość, jeśli $company nie pasuje do żadnego warunku
     }
}


 # Ustalamy ścieżki, gdzie użytkownik ma się znajdować oraz ścieżki do poszczególnych OU
 # Następnie sprawdzamy czy dane OU istnieje, jeżeli nie to takie tworzymy.
    if( $firma -eq "_Zarząd"){
	$sciezka = "OU=Users,OU=$firma,DC=AreoTrans,DC=local"
                $sciezka2 = "OU=$firma,DC=AreoTrans,DC=local"

                if (Get-ADOrganizationalUnit -Filter {Name -eq "Users"} -SearchBase $sciezka2) {
                Write-Host "OU: Users już istnieje"
                 } else {
                New-ADOrganizationalUnit -Name "Users" -Path $sciezka2
                Write-Host "Utworzono nowe OU: Users"
        }

    }
    else{
	$sciezka = "OU=Users,OU=$obszar,OU=$firma,DC=AreoTrans,DC=local"
                $sciezka2 = "OU=$firma,DC=AreoTrans,DC=local"
	$sciezka3 = "OU=$obszar, OU=$firma,DC=AreoTrans,DC=local"
    
        if (Get-ADOrganizationalUnit -Filter {Name -eq $obszar} -SearchBase $sciezka2) {
            Write-Host "OU: $obszar już istnieje"
        } else {
            New-ADOrganizationalUnit -Name $obszar -Path $sciezka2
            Write-Host "Utworzono nowe OU: $obszar"
        }

        if (Get-ADOrganizationalUnit -Filter {Name -eq "Users"} -SearchBase $sciezka3) {
            Write-Host "OU: Users już istnieje"
        } else {
            New-ADOrganizationalUnit -Name "Users" -Path $sciezka3
            Write-Host "Utworzono nowe OU: Users"
        }
    }
    


   # Sprawdzamy czy użytkownik istnieje w domenie Active Directory
    if (Get-ADUser -Filter {SamAccountName -eq $logonName}) {
        Write-Host "Użytkownik $logonName już istnieje."


    } else {


	# Jeżeli nie istnieje Kamila.Iwon, czyli użytkownik który musi mieć oddzielny sposób utworzenia (bez atrybutu manager)
	if ($logonName -eq "Kamila.Iwon") {


                # Dodajemy nowego użytkownika do domeny (bez atrybutu manager)
    	New-ADUser -Name $nazwiskoimie -GivenName $imie -Surname $nazwisko -SamAccountName $logonName -AccountPassword (ConvertTo-SecureString -AsPlainText $password -Force) -Enabled $true -PassThru -CannotChangePassword $true -PasswordNeverExpires $true -Path $sciezka -EmailAddress $email -EmployeeID $id -Office $office -Title $jobTitle -Department $department -Company $company -Street $adres -OfficePhone $telefon -City $miasto -Country $kraj -PostalCode $kod -UserPrincipalName $email

	} else {
               # Dodajemy nowego użytkownika do domeny (z atrybutem manager)
     New-ADUser -Name $nazwiskoimie -GivenName $imie -Surname $nazwisko -SamAccountName $logonName -AccountPassword (ConvertTo-SecureString -AsPlainText $password -Force) -Enabled $true -PassThru -CannotChangePassword $true -PasswordNeverExpires $true -Path $sciezka -DisplayName $nazwiskoimie -EmailAddress $email -EmployeeID $id -Office $office -Title $jobTitle -Department $department -Company $company -Street $adres -Manager $przelozony -OfficePhone $telefon -City $miasto -Country $kraj -PostalCode $kod -UserPrincipalName $email
	}

	Write-Host "Użytkownik $logonName został dodany."
    }


    # ustalamy ścieżkę do przechowywania grup
    $groupsOU = "OU=_Groups,DC=AreoTrans,DC=local"


    # Sprawdzamy czy atrybut $grupa1 nie jest pusty
    if ($grupa1 -ne ""){
        $groupsOU = "OU=_Groups,DC=AreoTrans,DC=local"

	# Jeżeli atrybut nie jest pusty, a taka grupa istnieje to dodajemy do niej danego usera
        if (Get-ADGroup -Filter {Name -eq $grupa1} -SearchBase $groupsOU) {
            Add-ADGroupMember -Identity $grupa1 -Members $logonName

	# Jeżeli atrybut nie jest pusty, a taka grupa nie istnieje to najpierw tworzymy ją, a potem dodajemy do niej danego usera
        } else {
	    New-ADGroup -Name $grupa1 -GroupScope Global -Path $groupsOU
            Add-ADGroupMember -Identity $grupa1 -Members $logonName
        }
    }



    # Sprawdzamy czy atrybut $grupa2 nie jest pusty   
    if ($grupa2 -ne ""){

	# Jeżeli atrybut nie jest pusty, a taka grupa istnieje to dodajemy do niej danego usera
        if (Get-ADGroup -Filter {Name -eq $grupa2} -SearchBase $groupsOU) {
            Add-ADGroupMember -Identity $grupa2 -Members $logonName

	# Jeżeli atrybut nie jest pusty, a taka grupa nie istnieje to najpierw tworzymy ją, a potem dodajemy do niej danego usera
        } else {
	    New-ADGroup -Name $grupa2 -GroupScope Global -Path $groupsOU
            Add-ADGroupMember -Identity $grupa2 -Members $logonName
        }
    }
    


    # Sprawdzamy czy atrybut $grupa3 nie jest pusty   
    if ($grupa3 -ne ""){

	# Jeżeli atrybut nie jest pusty, a taka grupa istnieje to dodajemy do niej danego usera
        if (Get-ADGroup -Filter {Name -eq $grupa3} -SearchBase $groupsOU) {
            Add-ADGroupMember -Identity $grupa3 -Members $logonName

	# Jeżeli atrybut nie jest pusty, a taka grupa nie istnieje to najpierw tworzymy ją, a potem dodajemy do niej danego usera
        } else {
	    New-ADGroup -Name $grupa3 -GroupScope Global -Path $groupsOU
            Add-ADGroupMember -Identity $grupa3 -Members $logonName
        }
    }
    
}
Write-Host "Skrypt został wykonany"