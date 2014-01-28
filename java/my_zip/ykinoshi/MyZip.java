package ykinoshi;

import java.util.Enumeration;

import org.apache.commons.compress.archivers.zip.ZipFile;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;

public class MyZip {
    
    public static void main(String[] args) throws Exception {
        
        ZipFile zipFile = new ZipFile(args[0], "Windows-31J");
        @SuppressWarnings("unchecked")
        Enumeration<? extends ZipArchiveEntry> entries = zipFile.getEntries();
        while (entries.hasMoreElements()) {
            ZipArchiveEntry ze = entries.nextElement();
            System.out.println(ze.getName());
        }
    }
}

