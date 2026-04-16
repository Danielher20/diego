import sys
from datetime import datetime

from reportlab.lib import colors
from reportlab.lib.pagesizes import letter, landscape
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import inch
from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle


def read_data(path):
    data = {
        "evals": [],
        "alumnos": [],
    }
    with open(path, "r", encoding="utf-8") as fh:
        for raw in fh:
            parts = raw.rstrip("\n").split("|")
            key = parts[0]
            if key == "EVAL":
                if len(parts) == 5:
                    data["evals"].append({
                        "nombre": parts[1],
                        "tipo": parts[2],
                        "peso": parts[3],
                        "nota": parts[4],
                    })
                else:
                    data["evals"].append({
                        "nombre": parts[1],
                        "tipo": parts[2],
                        "peso": parts[3],
                    })
            elif key == "ALUMNO":
                data["alumnos"].append(parts[1:])
            elif key == "PROMEDIO":
                data["promedio"] = parts[1]
                data["condicion"] = parts[2]
            else:
                data[key.lower()] = "|".join(parts[1:])
    return data


def styles():
    base = getSampleStyleSheet()
    return {
        "title": ParagraphStyle(
            "title",
            parent=base["Title"],
            fontName="Times-Bold",
            fontSize=18,
            leading=22,
            textColor=colors.black,
            alignment=0,
            spaceAfter=4,
        ),
        "subtitle": ParagraphStyle(
            "subtitle",
            parent=base["Normal"],
            fontName="Helvetica",
            fontSize=9,
            leading=12,
            textColor=colors.black,
        ),
        "small": ParagraphStyle(
            "small",
            parent=base["Normal"],
            fontName="Helvetica",
            fontSize=8,
            leading=10,
            textColor=colors.black,
        ),
    }


def table_style():
    return TableStyle([
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("FONTNAME", (0, 1), (-1, -1), "Helvetica"),
        ("FONTSIZE", (0, 0), (-1, -1), 8),
        ("TEXTCOLOR", (0, 0), (-1, -1), colors.black),
        ("LINEBELOW", (0, 0), (-1, 0), 0.7, colors.black),
        ("LINEBELOW", (0, 1), (-1, -1), 0.25, colors.lightgrey),
        ("ALIGN", (0, 0), (-1, -1), "CENTER"),
        ("ALIGN", (2, 1), (2, -1), "LEFT"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("TOPPADDING", (0, 0), (-1, -1), 6),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
    ])


def build_acta(data, out_path):
    st = styles()
    doc = SimpleDocTemplate(
        out_path,
        pagesize=landscape(letter),
        rightMargin=0.45 * inch,
        leftMargin=0.45 * inch,
        topMargin=0.35 * inch,
        bottomMargin=0.35 * inch,
    )

    story = [
        Paragraph(datetime.now().strftime("%d/%m/%Y, %I:%M %p").lower(), st["small"]),
        Paragraph("Resultados finales", st["title"]),
        Paragraph(
            f"{data.get('materia', '')} - {data.get('seccion', '')} - Periodo {data.get('periodo', '')}",
            st["subtitle"],
        ),
        Spacer(1, 0.25 * inch),
        Paragraph(f"Docente: {data.get('docente', '')}", st["subtitle"]),
        Spacer(1, 0.18 * inch),
    ]

    header = ["#", "Cedula", "Apellidos,\nNombres"]
    for idx, ev in enumerate(data["evals"], 1):
        header.append(f"E{idx}\n{float(ev['peso']):.0f}%")
    header.extend(["Definitiva", "Condicion"])

    rows = [header]
    for idx, alumno in enumerate(data["alumnos"], 1):
        cedula, nombres, apellidos, retirado, *rest = alumno
        notas = rest[:len(data["evals"])]
        definitiva = rest[len(data["evals"])]
        condicion = rest[len(data["evals"]) + 1]
        if retirado == "1":
            notas = ["RET"] * len(data["evals"])
        rows.append([idx, cedula, f"{apellidos},\n{nombres}", *notas, definitiva, condicion.title()])

    col_widths = [0.35 * inch, 0.95 * inch, 1.55 * inch]
    col_widths += [0.52 * inch] * len(data["evals"])
    col_widths += [0.8 * inch, 0.95 * inch]
    table = Table(rows, repeatRows=1, colWidths=col_widths)
    table.setStyle(table_style())
    story.append(table)
    doc.build(story)


def build_boleta(data, out_path):
    st = styles()
    doc = SimpleDocTemplate(
        out_path,
        pagesize=letter,
        rightMargin=0.6 * inch,
        leftMargin=0.6 * inch,
        topMargin=0.55 * inch,
        bottomMargin=0.55 * inch,
    )

    alumno = data["alumnos"][0]
    cedula, nombres, apellidos, retirado = alumno[:4]
    story = [
        Paragraph("Boleta de calificaciones", st["title"]),
        Paragraph(f"Materia: {data.get('materia', '')}", st["subtitle"]),
        Paragraph(f"Seccion: {data.get('seccion', '')} - Periodo {data.get('periodo', '')}", st["subtitle"]),
        Paragraph(f"Docente: {data.get('docente', '')}", st["subtitle"]),
        Paragraph(f"Alumno: {nombres} {apellidos}", st["subtitle"]),
        Paragraph(f"Cedula: {cedula}", st["subtitle"]),
        Spacer(1, 0.25 * inch),
    ]

    rows = [["Evaluacion", "Tipo", "Ponderacion", "Nota"]]
    for idx, ev in enumerate(data["evals"], 1):
        nota = "RETIRADO" if retirado == "1" else f"{float(ev['nota']):.2f}"
        rows.append([f"E{idx} - {ev['nombre']}", ev["tipo"], f"{float(ev['peso']):.2f}%", nota])
    table = Table(rows, repeatRows=1, colWidths=[2.2 * inch, 1.7 * inch, 1.1 * inch, 1.0 * inch])
    table.setStyle(table_style())
    story.append(table)
    story.append(Spacer(1, 0.2 * inch))
    story.append(Paragraph(f"Definitiva: {float(data.get('promedio', '0')):.2f} / 20", st["subtitle"]))
    story.append(Paragraph(f"Condicion: {data.get('condicion', '').title()}", st["subtitle"]))
    doc.build(story)


def main():
    if len(sys.argv) != 4:
        print("Uso: python pdf_report.py acta|boleta datos.txt salida.pdf")
        return 1

    kind, data_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
    data = read_data(data_path)
    if kind == "acta":
        build_acta(data, out_path)
    elif kind == "boleta":
        build_boleta(data, out_path)
    else:
        print("Tipo no reconocido")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
